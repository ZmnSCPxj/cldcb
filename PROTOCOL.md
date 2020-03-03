
* `give_recognition_code` 0
  * tlv: `recognition_code` 0, length 64
  * Client to server.
  * Associates the client ID with the given recognition code.
  * Server rejects this message if not recognized client ID.

* `request_recognition_codes` 2
  * Client to server.
  * Requests a list of client IDs and recognition codes.
  * Server always gives this message.

* `response_recognition_codes` 4
  * tlv: `cid_recognition_codes` 0, length n * 97
  * Server to client.
  * Returns the client IDs and recognition codes.
  * 0 or more are sent in response to
    `request_recognition_codes`.
* `response_recognition_codes_end` 6
  * Server to client.
  * Tells the client there are no more recognition codes.

--

* `ping` 8
  * tlv: `pointless_data` 0, length n
  * Either direction.
  * Pointless message to keep the TCP connection alive.
  * The receiver should reply with a `pong`.
* `pong` 9
  * tlv: `pointless_data` 0, length n
  * Either direction.
  * Pointless message to keep the TCP connection alive.
  * The receiver should ignore this message.

--

* `request_incremental` 32
  * tlv: `data_version` 0, length 4
  * Client to server.
  * Tells the server that a new incremental update
    is available.
  * Server rejects this message if not recognized client ID.
* `response_incremental` 34
  * Server to client.
  * Tells the client to send the incremental update
    data.
  * The client may now send `incremental_chunk` and
    `incremental_end` messages.
  * Possible response to `request_incremental`.
* `response_reupload` 36
  * Server to client.
  * Tells the client that it has to send a reupload
    of the current state first.
  * The client may now send `reupload_chunk` and
    `reupload_end` messages.
  * Possible response to `request_incremental`.

* `reupload_chunk` 52
  * tlv: `chunk` 0, length n
  * Client to server.
  * Provides a chunk of reupload data to the server.
  * Server rejects this message if it is not currently
    asking for a reupload update.
* `reupload_end` 54
  * Client to server.
  * Tells the server that there is no more reupload data.
* `reupload_acknowledge` 56
  * Server to client.
  * Tells the client that the server has cached the
    reupload data and it should now send the incremental
    data.
  * The client may now send `incremental_chunk` and
    `incremental_end` messages.
  * Response to `reupload_end`.

* `incremental_chunk` 68
  * tlv: `chunk` 0, length n
  * Client to server.
  * Provides a chunk of incremental data to the server.
  * Server rejects this message if it is not currently
    asking for an incremental update.
* `incremental_end` 70
  * Client to server.
  * Tells the server that there is no more incremental
    data to send.
* `incremental_acknowledge` 72
  * Server to client.
  * Tells the client that the server has backed up the
    incremental data (in particular, synched the disk)
    as well as any previous reupload data.
  * Response to `incremental_end`.

--

* `request_backup_data` 112
  * tlv: `cid` 0, length 33
    * The client backup data that the client wants to
      fetch.
  * Client to server.
  * Asks the server to return the backup data.
  * The server will start by sending
    `response_backedup_reupload_chunk` and so on.
* `response_backedup_reupload_chunk` 114
  * tlv: `chunk` 0, length n
  * Server to client.
  * Gives the client a chunk of the reupload data.
* `response_backedup_reupload_end` 116
  * Server to client.
  * Tells the client the reupload data has finished.
* `response_backedup_incremental_new` 118
  * tlv: `data_version` 0, length 4
  * Server to client.
  * Tells the client that a new increment is starting.
    Succeeding `response_backedup_increment_chunk`
    messages will belong to the given `data_version`.
* `response_backedup_incremental_chunk` 120
  * tlv:: `chunk` 0, length n
  * Server to client.
  * Gives the client a chunk of the increment data.
* `response_backedup_incremental_endall` 122
  * Server to client.
  * Tells the client that all of the incremental
    data has been sent.

