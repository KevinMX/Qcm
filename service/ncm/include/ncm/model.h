#pragma once

#include <chrono>

#include "json_helper/helper.h"
#include "ncm/type.h"

namespace ncm
{
namespace model
{

struct Time {
    using time_point = std::chrono::system_clock::time_point;
    i64        milliseconds { 0 };
    time_point point;
};

using Bool = std::variant<i64, bool>;

enum class SongFee
{
    Free                 = 0,
    Vip                  = 1,
    DigitalAlbum         = 4,
    Free128k             = 8,
    OnlyDownloadWithPaid = 16,
    OnlyOnlineWithPaid   = 32
};

struct Song {
    struct Ar {
        i64                                     id { 0 };
        std::string                             name;
        std::optional<std::vector<std::string>> alia;
        std::optional<std::string>              picUrl;
        std::optional<i64>                      picId { 0 };
    };
    struct Al {
        i64                        id { 0 };
        std::optional<std::string> name;
        std::string                picUrl;
        std::optional<i64>         picId { 0 };
    };

    struct Quality {
        i64 br;
        i64 size;
        i64 sr;
    };

    struct Privilege {
        i64         id { 0 };
        SongFee     fee;
        i64         payed;
        i64         st;
        i64         pl;    // 999000,
        i64         dl;    // 999000,
        i64         sp;    // 7,
        i64         cp;    // 1,
        i64         subp;  // 1,
        bool        cs;    // false,
        i64         maxbr; // 999000,
        i64         fl;    // 320000,
        bool        toast;
        i64         flag; // 128,
        bool        preSell;
        i64         playMaxbr;          // 999000,
        i64         downloadMaxbr;      // 999000,
        std::string maxBrLevel;         // "lossless",
        std::string playMaxBrLevel;     // "lossless",
        std::string downloadMaxBrLevel; // "lossless",
        std::string plLevel;            // "lossless",
        std::string dlLevel;            // "lossless",
        std::string flLevel;            // "exhigh",
                                        // rscl null,

        /*
        songMaxBr plLevel       dlLevel
        值 	      音质 	        比特率
        dobly     杜比 	        无
        hires     hires         1999
        lossless  无损 	        999
        exhigh    极高 	        320
        standard  标准 	        128
        none      不能播放/下载	0
        */

        /*
        songFee
        值 	说明            详细描述
        0 	免费            免费歌曲
        1 	会员            普通用户无法免费收听下载；会员可收听和下载所有音质
        4 	数字专辑        所有用户只能在商城购买数字专辑后，才能收听下载
        8 	128K            普通用户可免费收听128k音质，但不能下载；会员可收听和下载所有音质
        16 	只能付费下载
普通用户只能付费下载后使用，不提供在线收听；会员只能下载后使用，不能在线收听
        32  只能付费播放 普通用户只能付费后收听，不能下载；会员可以直接收听，但不能下载
        */
    };

    std::vector<Ar> ar;
    Al              al;
    i64             st;
    // std::optional<std::string> noCopyrightRcmd; // null, str, object
    i64                        rtype;
    i64                        pst;
    std::vector<std::string>   alia;
    i64                        pop;
    std::optional<std::string> rt;
    i64                        mst;
    i64                        cp;
    std::string                cf;
    Time                       dt;
    i64                        ftype;
    i64                        no;
    i64                        fee;
    i64                        mv;
    i64                        t;
    i64                        v;
    std::optional<Quality>     h;
    std::optional<Quality>     m;
    std::optional<Quality>     l;
    std::optional<Quality>     sq;
    std::optional<Quality>     hr;
    std::string                cd;
    std::string                name;
    i64                        id { 0 };

    std::optional<Privilege> privilege;

    // a	null
    // rtUrls	[]
    // songJumpInfo	null
    // rurl	null
    // crbt	null
    // rtUrl	null
    // djId	0
};

struct SongB {
    std::string name;
    i64         id { 0 };
    // position	0
    // alias	[]
    i64         status { 0 };
    i64         fee { 0 };
    i64         copyrightId { 0 };
    std::string disc;
    // no	0
    std::vector<Song::Ar> artists;
    Song::Al              album;
    bool                  starred { false };
    i64                   popularity { 0 };
    i64                   score { 5 };
    i64                   starredNum { 0 };
    Time                  duration;
    i64                   playedNum { 0 };
    // dayPlays	0
    i64 hearTime { 0 };
    // ringtone	null
    // crbt	null
    // audition	null
    // copyFrom	""
    std::string commentThreadId;
    // rtUrl	null
    i64 ftype { 0 };
    // rtUrls	[]
    i64 copyright { 0 };
    // transName	null
    // sign	null
    // mark	0
    // noCopyrightRcmd	null
    // hMusic	{…}
    // mMusic	{…}
    // lMusic	{…}
    // bMusic	{…}
    // mp3Url	null
    // rtype	0
    // rurl	null
    // mvid	0
};

struct Artist {
    // topicPerson	0
    bool                       followed {false};
    std::vector<std::string>   alias;
    std::string                trans;
    i64                        musicSize {0};
    i64                        albumSize {0};
    std::optional<std::string> briefDesc;
    std::string                picUrl;
    std::string                img1v1Url;
    std::string                name;
    i64                        id { 0 };
    // std::string              picId_str;
    // std::string              img1v1Id_str;
};

struct Album {
    std::vector<Song>          songs;
    bool                       paid;
    bool                       onSale;
    i64                        mark;
    i64                        companyId;
    std::string                blurPicUrl;
    std::vector<std::string>   alias;
    std::vector<Artist>        artists;
    i64                        copyrightId;
    Artist                     artist;
    std::optional<std::string> briefDesc;
    Time                       publishTime;
    std::optional<std::string> company;
    std::string                picUrl;
    std::string                commentThreadId;
    std::optional<std::string> description;
    std::string                tags;
    i64                        status;
    std::string                subType;
    std::string                name;
    i64                        id { 0 };
    std::string                type;
    i64                        size;
    std::string                picId_str;
    // awardTags	null
    // info	{…}
};

struct Playlist {
    i64         id { 0 };
    std::string name;
    // coverImgId	109951167805071570
    std::string coverImgUrl;
    // coverImgId_str	"109951167805071571"
    // adType	0
    i64                userId;
    std::optional<i64> status;
    // opRecommend	false
    // highQuality	false
    // newImported	false

    i64 trackCount;
    i64 specialType;
    // privacy	0
    // trackUpdateTime	1678022963095
    std::optional<std::string> commentThreadId;
    i64                        playCount;
    // trackNumberUpdateTime	1678018138230
    // subscribedCount	4
    // cloudTrackCount	1
    // ordered	true
    std::optional<std::string>              description;
    std::optional<Time>                     createTime;
    std::optional<Time>                     updateTime;
    std::optional<std::vector<std::string>> tags;
    // updateFrequency	null
    // backgroundCoverId	0
    // backgroundCoverUrl	null
    // titleImage	0
    // titleImageUrl	null
    // englishTitle	null
    // officialPlaylistType	null
    // copied	false
    // relateResType	null
    // subscribers	[…]
    std::optional<bool> subscribed;
    // creator	{…}
    std::optional<std::vector<Song>> tracks;
    // videoIds	null
    // videos	null
    // trackIds	[…]
    // bannedTrackIds	null
    std::optional<i64> shareCount;
    std::optional<i64> commentCount;
    // remixVideo	null
    // sharedUsers	null
    // historySharedUsers	null
    // gradeStatus	"NONE"
    // score	null
    // algTags	null
};

struct User {
    // locationInfo	null
    // liveInfo	null
    // anonym	0
    // commonIdentity	null
    // avatarDetail	null
    i64         userType;
    std::string avatarUrl;
    bool        followed;
    // mutual	false
    // remarkName	null
    // socialUserId	null
    // vipRights	{…}
    std::string nickname;
    // authStatus	0
    // expertTags	null
    // experts	null
    i64 vipType;
    i64 userId;
    // target	null
};

struct Comment {
    User user;
    // beReplied	[]
    // pendantData	null
    // showFloorComment	null
    i64                        status;
    i64                        commentId;
    std::string                content;
    std::optional<std::string> richContent;
    // contentResource	null
    Time time;
    // timeStr	"08-10"
    // needDisplayTime	true
    i64 likedCount;
    // expressionUrl	null
    // commentLocationType	0
    // parentCommentId	0
    // decoration	{}
    // repliedMark	null
    // grade	null
    // userBizLevels	null
    // ipLocation	{…}
    bool owner;
    Bool liked;
};

struct Djradio {
    std::string category;
    std::string secondCategory;
    bool        buyed { false };
    i64         price { 0 };
    i64         originalPrice { 0 };
    // discountPrice":null,
    i64 purchaseCount { 0 };
    // std::string lastProgramName;
    // videos":null,
    bool finished { false };
    // underShelf":false,
    // liveInfo":null,
    i64  playCount { 0 };
    bool privacy { false };
    // icon":null,
    // manualTagsDTO":null,
    // descPicList":null,
    // replaceRadioId":0,
    // replaceRadio":null,
    Time lastProgramCreateTime;
    // shortName":null,
    std::string intervenePicUrl;
    i64         picId { 0 };
    i64         categoryId;
    // taskId":0,
    i64                 programCount { 0 };
    i64                 subCount { 0 };
    std::optional<bool> dynamic { false };
    i64                 radioFeeType { 0 };
    std::string         picUrl;
    i64                 lastProgramId { 0 };
    i64                 feeScope { 0 };
    // intervenePicId":19115009649278426,
    std::string name;
    i64         id { 0 };
    std::string desc;
    model::Time createTime;
    // rcmdtext":null,
    // newProgramCount":0
};

struct Program {
    SongB mainSong;
    // songs	null
    // dj	{…}
    std::string blurCoverUrl;
    // radio	{…}
    Time duration;
    // authDTO	null
    bool buyed { false };
    // programDesc	null
    // h5Links	null
    bool canReward { false };
    i64  auditStatus { 0 };
    // videoInfo	null
    i64 score { 0 };
    // liveInfo	null
    // alg	null
    // disPlayStatus	null
    i64                        auditDisPlayStatus { 1 };
    std::optional<std::string> categoryName;
    std::optional<std::string> secondCategoryName;
    bool                       existLyric { false };
    // djPlayRecordVo	null
    // recommended	false
    // icon	null
    // additionIconList	null
    // adIconInfo	null
    // replaceVoiceId	0
    // replaceResource	null
    // smallLanguageAuditStatus	0
    i64 pubStatus { 1 };
    i64 programFeeType { 0 };
    i64 mainTrackId { 0 };
    // titbits	null
    i64         feeScope { 0 };
    std::string coverUrl;
    bool        reward { false };
    i64         subscribedCount { 0 };
    // trackCount	0
    // titbitImages	null
    bool isPublish { false };
    i64  coverId { 0 };
    bool privacy { false };
    // channels	[]
    i64         categoryId { 0 };
    std::string commentThreadId;
    i64         listenerCount { 0 };
    // createEventId	0
    i64  serialNum { 36 };
    Time scheduledPublishTime;
    // bdAuditStatus	2
    i64         secondCategoryId { 0 };
    std::string name;
    i64         id { 0 };
    std::string description;
    Time        createTime;
    i64         shareCount { 0 };
    bool        subscribed { false };
    i64         likedCount { 0 };
    i64         commentCount { 0 };
};

JSON_DEFINE(Song);
JSON_DEFINE(SongB);
JSON_DEFINE(Artist);
JSON_DEFINE(Album);
JSON_DEFINE(Playlist);
JSON_DEFINE(Time);
JSON_DEFINE(Comment);
JSON_DEFINE(User);
JSON_DEFINE(Djradio);
JSON_DEFINE(Program);

} // namespace model

} // namespace ncm

template<>
struct Convert<bool, ncm::model::Bool> {
    Convert(bool& out, const ncm::model::Bool& i) {
        std::visit(
            [&out](auto v) {
                out = (bool)v;
            },
            i);
    }
};