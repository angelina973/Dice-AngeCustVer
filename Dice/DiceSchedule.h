#pragma once

/*
 * Copyright (C) 2019-2022 String.Empty
 * 处理定时事件
 * 处理不能即时完成的指令
 * 2022/1/13: 冷却计时
 * 2022/2/05: 今日数据类型扩展
 */

#include <string>
#include <map>
#include <unordered_map>
#include "DiceMsgSend.h"
#include "json.hpp"
#include "DiceAttrVar.h"
#include "STLExtern.hpp"

using std::string;
using std::map;
using std::unordered_map;
using std::shared_ptr;

struct DiceJobDetail : public std::enable_shared_from_this<DiceJobDetail> {
	chatInfo fromChat;
	time_t fromTime = time(nullptr);
	size_t cntExec{ 0 };
	//临时变量库
	AttrObject vars;
	DiceJobDetail(const AttrVars& vars, bool isFromSelf = false);
	DiceJobDetail(const AttrVars& var, chatInfo ct = {}) :vars(var), fromChat(ct){}
	virtual void reply(const char*, bool = true) {}
	virtual void reply(const string&, bool = true) {}
	AttrVar& operator[](const char* key){
		return vars[key];
	}
	bool operator<(const DiceJobDetail& other)const {
		return vars.get_str("cmd") < other.vars.get_str("cmd");
	}
};
extern AttrIndexs MsgIndexs;

/*
class DiceJob : public DiceJobDetail {
	enum class Renum { NIL, Retry_For, Retry_Until };
public:
	DiceJob(DiceJobDetail detail) :DiceJobDetail(detail) {}
	Renum ren = Renum::NIL;
	void exec();
	void echo(const std::string&);
	void reply(const std::string&);
	void note(const std::string&, int);
};
*/

struct CDQuest {
	chatInfo chat;
	string key;
	time_t cd;
	CDQuest(const chatInfo& ct, const string& k, time_t t) :chat(ct), key(k), cd(t) {}
};
enum class CDType :size_t { Chat, User, Global };
struct CDConfig {
	CDType type;
	string key;
	time_t cd;
	CDConfig(const CDType& ct, const string& k, time_t t) :type(ct), key(k), cd(t) {}
	static enumap<string> eType;
};

class DiceScheduler {
	//事件冷却期
	unordered_map<string, time_t> untilJobs;
	unordered_map<chatInfo, unordered_map<string, time_t>> cd_timer;
public:
	void start();
	void end();
	void push_job(const AttrObject&);
	void push_job(const char*, bool = false, const AttrVars& = {});
	void add_job_for(unsigned int, const AttrObject&);
	void add_job_for(unsigned int, const char*);
	void add_job_until(time_t, const AttrObject&);
	void add_job_until(time_t, const char*);
	bool is_job_cold(const char*);
	void refresh_cold(const char*, time_t);
	bool cnt_cd(const vector<CDQuest>&, const vector<CDQuest>&);
};
inline DiceScheduler sch;

typedef void (*cmd)(AttrObject&);

//今日记录
class DiceToday {
	tm stToday;
	unordered_map<string, int>cntGlobal;
	unordered_map<long long, AttrObject>UserInfo;
public:
	unordered_map<chatInfo, unordered_map<string, int>> counter;
	DiceToday() {
		load();
	}
	void load();
	void save();
	void set(long long qq, const string& key, const AttrVar& val);
	void inc(const string& key) { cntGlobal[key]++; save(); }
	//void inc(long long qq, const string& key, int cnt = 1) { cntUser[qq][key] += cnt; save(); }
	unordered_map<long long, AttrObject>& getUserInfo() { return UserInfo; }
	int& get(const string& key) { return cntGlobal[key]; }
	AttrObject& get(long long uid) { return UserInfo[uid]; }
	AttrVar& get(long long uid, const string& key) { return UserInfo[uid][key]; }
	AttrVar* get_if(long long qq, const string& key) {
		if (UserInfo.count(qq) && UserInfo[qq].has(key))
			return &UserInfo[qq][key];
		else return nullptr;
	}
	int getJrrp(long long qq);
	size_t cntUser() { return UserInfo.size(); }
	void daily_clear();
};
inline std::unique_ptr<DiceToday> today;

string printTTime(time_t tt);