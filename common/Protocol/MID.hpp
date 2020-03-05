#ifndef CLDCB_COMMON_PROTOCOL_MID_HPP
#define CLDCB_COMMON_PROTOCOL_MID_HPP

#include<cstdint>

namespace Protocol { namespace MID {

/* Message IDs.  */
enum Type
/* Lets an amnesiac client figure out the
 * client ID it used before.
 */
{ give_recognition_code = 0
, request_recognition_codes = 2
, response_recognition_codes = 4
, response_recognition_codes_end = 6

/* App-level keepalive.  */
, ping = 8
, pong = 9

/* Normal plugin `db_write` behavior.  */
, request_incremental = 32
, response_reupload = 36
, acknowledge_upload = 38
, incremental_chunk = 68
, incremental_end = 70
, reupload_chunk = 52
, reupload_end = 54

/* Recovery, after the client has figured out its
 * previous client ID.
 */
, request_backup_data = 112
, response_backedup_reupload_chunk = 114
, response_backedup_reupload_end = 116
, response_backedup_incremental_new = 118
, response_backedup_incremental_chunk = 120
, response_backedup_incremental_endall = 122
};

}}

#endif /* CLDCB_COMMON_PROTOCOL_MID_HPP */
