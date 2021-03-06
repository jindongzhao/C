//============================================================================
// Name        : talker1.cpp
// Author      : zjd
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <unistd.h>
#include "bo/Message.h"
#include "bo/User.h"
#include "interface/Sender.h"
#include "comps/email/QqSender.h"
#include "interface/Receiver.h"
#include "comps/email/E126Receiver.h"
#include <exception>

using namespace std;
using namespace ns_talker;

int main() {
	cout<<"请输入你的名称"<<endl;
	User me;
	string myName;
	cin>>myName;
	me.setId(myName);

	cout<<"请输入对方的名称"<<endl;
	User other;
	string otherName;
	cin>>otherName;
	other.setId(otherName);

	//初始化sender
	Sender* sender = new QqSender();
	sender->init(me,other);

	//初始化receiver
	Receiver* receiver = new E126Receiver();
	receiver->init(me);

	//接收未读消息
	Message msg = receiver->receive();
	if(msg.getContent().size() > 0){
		cout<<msg.getFromUserId()<<" 说："<<msg.getContent()<<endl;
	}

	while(true){
		cout<<"我说(输入exit退出)："<<endl;
		string words;
		cin>>words;

		if(words == "exit"){
			Message msg;
			msg.setContent("我下线了，byebye~");
			sender->send(msg);
			delete sender;		//回收sender对象
			delete receiver;	//回收receiver对象
			return 0;	//自然结束，如果用exit则不会调用user的析构函数
		}

		//发送消息
		Message msg;
		msg.setContent(words);
		sender->send(msg);

		cout<<"【消息已发送，等待对方回应...】"<<endl;
		while(true){
			sleep(3);
			try{
				Message msg = receiver->receive();
				if(msg.getContent().size() > 0){
					cout<<msg.getFromUserId()<<" 说："<<msg.getContent()<<endl;
					break;
				}
			}catch (exception e) {
				cout<<e.what()<<endl;
			}
		}
	}
	return 0;
}
