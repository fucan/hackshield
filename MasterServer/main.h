#ifndef _MAIN_H_
#define _MAIN_H_

#include "..\include.h"
#include "..\ghp_NET\mongoose.h"
#include "..\ghp_UTILS\ghp_LOG.h"
#include "..\ghp_UTILS\ghp_SYSTEM.h"
#include "..\ghp_UTILS\tinyxml2\tinyxml2.h"
#include "..\ghp_UTILS\hiberlite\hiberlite.h"

#define ADMIN_KEY "ZdDSP2EXXnRDNWbfpEqSHvpv"
#define SUB_KEY   "UO9OG6K3VK"

class Server
{
	friend class hiberlite::access;
	template<class Archive>
	void hibernate(Archive & ar)
	{
		ar & HIBERLITE_NVP(c_ref);
		ar & HIBERLITE_NVP(srvname);
		ar & HIBERLITE_NVP(srvip);
		ar & HIBERLITE_NVP(expire);
		ar & HIBERLITE_NVP(players);
    }
public:
	std::string c_ref;
	std::string srvname;
	std::string srvip;
	std::string expire;
	int players;
}; HIBERLITE_EXPORT_CLASS(Server)

class Cliente
{
	friend class hiberlite::access;
	template<class Archive>
	void hibernate(Archive & ar)
	{
		ar & HIBERLITE_NVP(name);
		ar & HIBERLITE_NVP(email);
		ar & HIBERLITE_NVP(servers);
    }
public:
	std::string name;
	std::string email;
	std::vector<Server> servers;
}; HIBERLITE_EXPORT_CLASS(Cliente)

#endif