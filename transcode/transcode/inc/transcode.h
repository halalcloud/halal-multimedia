#ifndef TRANSCODE_H_INCLUDED
#define TRANSCODE_H_INCLUDED
const char transcode_command_key[] = "command"; ////value type : transcode_command_value
const char tcv_regist[]  = "regist";                  //regist client
const char tcv_create[]  = "create";                      //create task
const char tcv_cancel[]  = "cancel";                      //cancel task
const char tcv_quit[]    = "quit";                        //quit process
const char tcv_status[]  = "status";                      //get status
const char tcv_return[]  = "return";                    //return task result
const char tcv_warring[] = "warring";                     //task warring
const char tcv_error[]   = "error";                       //task error
const char tcv_input[]   = "input";                      //get task input info
const char tcv_message[] = "message";                     //task warring

const char tcv_regist_app_key[] = "app"; //value type : uint16_t
const char tcv_regist_port_key[] = "port"; //value type : uint16_t
const char tcv_regist_dump_path_key[] = "dump_path"; //value type : char*

const char tcv_task_key[] = "task";          //task id value type: char*

const char tcv_create_json_key[] = "json";         //value type : char*

enum transcode_status
{
    ts_create = 0,
    ts_regist,
    ts_running,
    ts_cancel,
    ts_success,
    ts_fail,
    ts_free
};

const char tcv_status_key[] = "status";               // value type : transcode_status
const char tcv_descr_key[] = "descr";           //value type : char*

#endif // TRANSCODE_H_INCLUDED
