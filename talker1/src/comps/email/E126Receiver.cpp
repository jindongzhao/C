/*
 * E126Receiver.cpp
 *
 *  Created on: 2014年12月23日
 *      Author: zjd
 */

#include "E126Receiver.h"
#include "../../common/CommonHeader.h"
#include "../../interface/Receiver.h"
#include "../../bo/User.h"
#include "../../bo/Message.h"

using namespace std;

namespace ns_talker {

E126Receiver::E126Receiver() {
}

E126Receiver::~E126Receiver() {
//	cout<<"xi gou receiver."<<endl;
}

void E126Receiver::init(User& me) {
	this->me = &me;
}

Message E126Receiver::receive() {
	Message msg;
	char receiveBuf[BUFSIZ];

	//初始化socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		throw std::runtime_error("e126 receiver 初始化socket异常 \n");
	}

	//连接服务端
	struct sockaddr_in server;
	struct hostent* hp;
	server.sin_family = AF_INET;
	hp = gethostbyname("pop.126.com");
	if (hp == (struct hostent *) 0) {
		std::cerr << "e126 receiver 服务器地址获取失败！" << std::endl;
	}
	memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
	server.sin_port = htons(110);
	if (connect(sock, (sockaddr *) &server, sizeof server) == -1) {
		std::cerr << "e126 receiver 连接服务器失败！" << std::endl;
	}
	Util::receiveSocketMsg(sock, receiveBuf);
//	std::cout << "e126 receiver 连接服务器成功。服务器返回：" << receiveBuf << std::endl;

	//登录
	//发送用户名
	Util::sendSocketMessage(sock,string("user jindongzhao@126.com\r\n"));//邮箱地址
    Util::receiveSocketMsg(sock, receiveBuf);
//	std::cout << "e126 receiver发送登录用户名。服务器返回:" <<receiveBuf << std::endl;

	//发送密码
	Util::sendSocketMessage(sock,string("pass ")+string(Util::Base64Decode(string("emhhbmdqdW4=")))+string("\r\n"));
	Util::receiveSocketMsg(sock, receiveBuf);
//	std::cout << "e126 receiver发送登录密码。服务器返回:" <<receiveBuf << std::endl;

	//获取邮件总数
	string countStr = Util::subStr(string(receiveBuf),string("OK "),string(" message"));
	int countInt = atoi(countStr.data());


	//获取最后3封邮件，如果是发给me的，则返回msg
	for(int i=0;i<3;i++){
		int idx = countInt - i;
		Util::sendSocketMessage(sock,"RETR  "+Util::itoa(idx)+"\r\n");
		Util::receiveSocketMsg(sock, receiveBuf);
	//	cout<<"e126 receiver 获取邮件内容。服务器返回："<<receiveBuf<<endl;

		string fromUserId = Util::subStr(string(receiveBuf),"^^^^^^^^^^","&&&&&&&&&&");
		string toUserId = Util::subStr(string(receiveBuf),"&&&&&&&&&&","##########");
		string content = Util::subStr(string(receiveBuf),">>>>>>>>>>","<<<<<<<<<<");
//		cout<<"邮件来自："<<fromUserId<<";发给："<<toUserId<<";内容："<<content<<endl;

		//如果是发给我的邮件，则删除、退出邮件回话，并返回msg
		if(me->getId() == toUserId){
			Util::sendSocketMessage(sock,"dele  "+Util::itoa(idx)+"\r\n");
			Util::receiveSocketMsg(sock, receiveBuf);
	//		cout<<"删除第"<<countStr<<"封邮件。服务器返回："<<receiveBuf<<endl;

			//退出
			Util::sendSocketMessage(sock,"quit  \r\n");
			Util::receiveSocketMsg(sock, receiveBuf);

			//构造msg
			msg.setContent(content);
			msg.setFromUserId(fromUserId);
			msg.setToUserId(toUserId);
			break;
		}
	}

	return msg;
}

} /* namespace ns_talker */
