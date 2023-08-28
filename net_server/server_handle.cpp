#include "server_handle.h"

queue<char*>* SHandle::msg_queue_ = new queue<char*>();
ThreadPool* SHandle::threadPool_ = new ThreadPool();

/*
* @brief 解析接收到的数据，根据mode再次选择对应handle
*/
void SHandle::handle_receive(shared_ptr<SSession> session,char* msgRecv){
    shared_ptr<Json> json = make_shared<Json>(msgRecv);
    cout<<"receive！"<<endl;
    switch (json->getInt("mode"))
    {
    case MSGMODE_MYSQL_QUERY_EXIST: //查询是否存在
        threadPool_->commit(handle_queryExist,session,json->getCharPtr("queryCmd"));
        break;
    case MSGMODE_REDIS_USER_SENDMSG:   //发送消息
        threadPool_->commit(handle_redis_sendMsg,session,json);
        break;
    case MSGMODE_REDIS_USER_SHOWMSG:  //显示用户沟通的消息
        cout<<"call0"<<endl;
        threadPool_->commit(handle_redis_showMsg,session,json);
        break;
    default:
        break;
    }
}

/*
* @brief 查询，是否存在
*/
void SHandle::handle_queryExist(shared_ptr<SSession> session,const char* queryCmd){
    Mysql* ms = new Mysql();
    ms->mysqlConnection();
    MYSQL_RES* queryRes = nullptr;
    queryRes = ms->mysqlQuery(queryCmd);
    bool isExist = mysql_num_rows(queryRes)>0?true:false;
    Json* json = new Json();
    json->appendInt("mode",1);
    json->appendBool("queryRes",isExist);
    string msg = MSG::packing(json);
    session->send(const_cast<char*>(msg.c_str()),msg.length());
    mysql_free_result(queryRes);
}

void SHandle::handle_redis_sendMsg(shared_ptr<SSession> session,shared_ptr<Json> json){
    RedisMSG::sendMessage(json->getCharPtr("fromUser"),json->getCharPtr("toUser"),json->serialization());
}

void SHandle::handle_redis_showMsg(shared_ptr<SSession> session,shared_ptr<Json> json){
    cout<<"call1"<<endl;
    string temp = RedisMSG::getUserMessage(json->getCharPtr("fromUser"));
    cout<<"call2"<<endl;
}