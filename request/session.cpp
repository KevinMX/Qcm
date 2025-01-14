#include <asio/deferred.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <iostream>

#include "session.h"
#include "session_p.h"

#include "request.h"
#include "request_p.h"

#include "response.h"
#include "response_p.h"

#include "curl_multi.h"

#include "connection.h"

using namespace request;

constexpr static auto POLL_TIMEOUT { std::chrono::milliseconds(1000) };
namespace sm = session_message;

namespace
{

template<typename T>
T get_curl_private(CURL* c) {
    T        easy { nullptr };
    CURLcode rc = curl_easy_getinfo(c, CURLINFO_PRIVATE, &easy);
    _assert_(! rc);
    return easy;
}

} // namespace

Session::Session(executor_type ex): m_p(std::make_unique<Private>(*this, ex)) {
    C_D(Session);
    asio::dispatch(d->m_poll_thread.get_executor(), [this, d]() {
        auto self = get_rc();
        asio::co_spawn(d->m_poll_thread.get_executor(), d->run(), asio::detached);
        asio::co_spawn(
            d->m_channel_with_notify->get_executor(),
            [d, self]() -> asio::awaitable<void> {
                for (;;) {
                    auto [ec, msg] = co_await d->m_channel_with_notify->async_receive(
                        asio::as_tuple(asio::use_awaitable));
                    bool stopped = std::get_if<sm::Stop>(&msg);
                    bool send_ok = d->m_channel->try_send(ec, msg);
                    // notify
                    d->m_curl_multi->wakeup();
                    if (stopped) break;
                    // if channel full, wait
                    if (! send_ok) {
                        co_await d->m_channel->async_send(ec, msg, asio::use_awaitable);
                    }
                }
                co_return;
            },
            asio::detached);
    });
}

Session::~Session() {
    C_D(Session);
    about_to_stop();
    d->m_poll_thread.join();
}

Session::executor_type& Session::get_executor() {
    C_D(Session);
    return d->m_ex;
}
asio::strand<Session::executor_type>& Session::get_strand() {
    C_D(Session);
    return d->m_strand;
}

asio::awaitable<bool> Session::perform(rc<Response>& rsp) {
    C_D(Session);
    auto& con = rsp->connection();
    rsp->prepare_perform();

    sm::ConnectAction msg {
        .con    = con.get_rc(),
        .action = sm::ConnectAction::Action::Add,
    };

    co_await channel().async_send(asio::error_code {}, msg, asio::use_awaitable);

    co_await con.async_wait_header(asio::use_awaitable);
    co_return true;
}

asio::awaitable<std::optional<rc<Response>>> Session::get(const Request& req) {
    C_D(Session);
    auto res = Response::make_response(req, Operation::GetOperation, shared_from_this());

    if (co_await perform(res)) co_return res;
    co_return std::nullopt;
}

asio::awaitable<std::optional<rc<Response>>> Session::post(const Request&     req,
                                                           asio::const_buffer buf) {
    C_D(Session);
    rc<Response> res = Response::make_response(req, Operation::PostOperation, shared_from_this());
    res->add_send_buffer(buf);

    if (co_await perform(res)) co_return res;
    co_return std::nullopt;
}

Session::Private::Private(Session& p, executor_type& ex) noexcept
    : m_p(p),
      m_curl_multi(std::make_unique<CurlMulti>()),
      m_ex(ex),
      m_strand(ex),
      m_poll_thread(1),
      m_channel(std::make_shared<channel_poll_type>(m_poll_thread.get_executor(), 1024)),
      m_channel_with_notify(std::make_shared<channel_type>(asio::make_strand(ex), 1024)),
      m_stopped(false) {};

void Session::load_cookie(std::filesystem::path p) {
    C_D(Session);
    d->m_curl_multi->load_cookie(p);
}
void Session::save_cookie(std::filesystem::path p) const {
    C_D(const Session);
    d->m_curl_multi->save_cookie(p);
}

void Session::test() { C_D(Session); }

Session::channel_type& Session::channel() {
    C_D(Session);
    return *(d->m_channel_with_notify);
}

rc<Session::channel_type> Session::channel_rc() {
    C_D(Session);
    return d->m_channel_with_notify;
}

void Session::about_to_stop() {
    C_D(Session);
    channel().try_send(asio::error_code {}, sm::Stop {});
}

void Session::Private::add_connect(const rc<Connection>& con) {
    auto ec = m_curl_multi->add_handle(con->easy());
    if (ec) {
        ERROR_LOG("{}", ec.message());
        return;
    }
    DEBUG_LOG("add {}", con->url());
    con->transfreing();
    m_connect_set.insert(con);
}
void Session::Private::remove_connect(const rc<Connection>& con) {
    DEBUG_LOG("end {}", con->url());
    m_curl_multi->remove_handle(con->easy());
    m_connect_set.erase(con);
}

asio::awaitable<void> Session::Private::run() {
    do {
        while (m_connect_set.empty() && ! m_stopped) {
            auto msg = co_await m_channel->async_receive(asio::use_awaitable);
            handle_message(msg);
        }
        while (m_channel->try_receive([this](auto, const auto& m) {
            handle_message(m);
        })) {
        }

        int running_connect { 0 };
        if (auto re = m_curl_multi->perform(running_connect); re) {
            ERROR_LOG("{}", re.message());
        };

        // check done before poll, to make sure multi has active sockets to poll
        auto infos = m_curl_multi->query_info_msg();
        for (auto& m : infos) {
            if (m.msg != CURLMSG_DONE) continue;
            auto con = get_curl_private<Connection*>(m.easy_handle)->get_rc();
            con->finish(m.result);
            remove_connect(con);
            running_connect--;
        }

        if (running_connect > 0) {
            if (auto re = m_curl_multi->poll(POLL_TIMEOUT); re) {
                ERROR_LOG("{}", re.message());
            }
        }

        if (m_connect_set.empty()) {
            DEBUG_LOG("all connection finished");
        }
    } while (! m_stopped);
    DEBUG_LOG("session stopped");
}

void Session::Private::handle_message(const SessionMessage& msg) {
    namespace sm = session_message;
    std::visit(overloaded { [this](sm::Stop) {
                               m_stopped = true;
                               while (m_connect_set.size()) {
                                   auto& con = *m_connect_set.begin();
                                   con->cancel();
                                   remove_connect(con);
                               }
                               m_connect_set.clear();
                           },
                            [this](const sm::ConnectAction& con_act) {
                                switch (con_act.action) {
                                    using enum sm::ConnectAction::Action;
                                case Add: add_connect(con_act.con); break;
                                case Cancel:
                                    con_act.con->cancel();
                                    remove_connect(con_act.con);
                                    break;
                                case Pause: con_act.con->easy().pause(CURLPAUSE_RECV); break;
                                case UnPause: con_act.con->easy().pause(CURLPAUSE_CONT); break;
                                default: break;
                                }
                            } },
               msg);
}
