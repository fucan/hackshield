#include "main.h"

hiberlite::Database db;
ghp_UTILS::Log masterLog(L"MasterServer.log", false);

bool IsClientAtDB(std::string email)
{
	std::vector<hiberlite::bean_ptr<Cliente>> v = db.getAllBeans<Cliente>();

	for (UINT i = 0; i < v.size(); i++)
	{
		if (v[i]->email == email)
			return true;
	}

	return false;
}

bool C_REFAtDB(std::string c_ref)
{
	std::vector<hiberlite::bean_ptr<Cliente>> v = db.getAllBeans<Cliente>();

	for (UINT i = 0; i < v.size(); i++)
	{
		for (UINT j = 0; j < v[i]->servers.size(); j++)
		{
			if (v[i]->servers[j].c_ref == c_ref)
			{
				return true;
			}
		}
	}

	return false;
}

std::string GetClientData(std::string c_ref)
{
	std::vector<hiberlite::bean_ptr<Cliente>> v = db.getAllBeans<Cliente>();

	for (UINT i = 0; i < v.size(); i++)
	{
		for (UINT j = 0; j < v[i]->servers.size(); j++)
		{
			if (v[i]->servers[j].c_ref == c_ref)
			{
				return v[i]->name;
			}
		}
	}

	return "";
}

int server_request(struct mg_connection * conn, enum mg_event ev)
{
	if (ev == MG_REQUEST)
	{
		std::string suri = conn->uri;
		// Ignora FAVICON
		if (suri == "/favicon.ico")
		{
			return MG_TRUE;
		}

		// Pega a C_REF
		char c_ref[48];
		char c[8];
		char p[8];
		mg_get_var(conn, "c_ref", c_ref, 48);
		mg_get_var(conn, "c", c, 8);
		mg_get_var(conn, "p", p, 8);
		
		// Pega os dados do cliente
		std::string ret_data;
		
		if (strlen(c_ref) > 0 && strlen(p) > 0 && strlen(c) == 0)
			ret_data = GetClientData(c_ref);

		// Verifica se os dados são válidos
		if (ret_data != "")
		{
			// Salva a quantidade de players
			/*if (strlen(players) > 0)
				SetClientData(c_ref, "players", players);
			
			// Log
			printf("REQ_OK >> %s : %s\n", conn->remote_ip, c_ref);
			masterLog.LineOut(true, "REQ_OK >> %s : %s", conn->remote_ip, c_ref);*/

			// Retorna os dados para o cliente
			mg_printf_data(conn, ret_data.c_str());
		}
		else
		{
			// Verifica se é algum comando do administrador
			char key[30]; std::string skey;
			char command[8]; std::string scommand;

			mg_get_var(conn, "key", key, 30);
			mg_get_var(conn, "c", command, 8);

			skey = key;
			scommand = command;

			if ((skey == ADMIN_KEY || skey == SUB_KEY) && scommand != "")
			{
				if (scommand == "addc")
				{
					char name[128];
					char email[128];

					mg_get_var(conn, "name", name, 128);
					mg_get_var(conn, "email", email, 128);
					
					if (!IsClientAtDB(email) && strlen(name) > 1 && strlen(email) > 1)
					{
						Cliente c;
						c.name = name;
						c.email = email;
						db.copyBean(c);

						printf("ADMIN_REQ_OK >> %s : addc - %s\n", conn->remote_ip, email);
						masterLog.LineOut(true, "ADMIN_REQ_OK >> %s : addc - %s", conn->remote_ip, email);
						mg_printf_data(conn, "OK!\nName: %s\nEmail: %s", name, email);
					}
					else
					{
						printf("ADMIN_REQ_ERROR >> %s : %s%s\n", conn->remote_ip, conn->uri, conn->query_string);
						masterLog.LineOut(true, "ADMIN_REQ_ERROR >> %s : %s%s", conn->remote_ip, conn->uri, conn->query_string);
					}
				}
				else if (scommand == "adds")
				{
					char email[128];
					char c_ref[128];
					char srvname[128];
					char srvip[128];
					
					mg_get_var(conn, "email", email, 128);
					mg_get_var(conn, "c_ref", c_ref, 128);
					mg_get_var(conn, "srvname", srvname, 128);
					mg_get_var(conn, "srvip", srvip, 128);

					if (IsClientAtDB(email) && !C_REFAtDB(c_ref) && strlen(email) > 1 && strlen(c_ref) > 1 && strlen(srvname) > 1 && strlen(srvip) > 1)
					{
						std::vector<hiberlite::bean_ptr<Cliente>> v = db.getAllBeans<Cliente>();

						for (UINT i = 0; i < v.size(); i++)
						{
							if (v[i]->email == email)
							{
								char expire[128];

								struct tm local;
								time_t raw;
								time(&raw);
								localtime_s(&local, &raw);
								sprintf_s(expire, "%02d/%02d/%02d", local.tm_mday + 1, local.tm_mon == 11 ? 0 : local.tm_mon + 2, local.tm_year + 1900);

								Server s;
								s.c_ref = c_ref;
								s.srvname = srvname;
								s.srvip = srvip;
								s.expire = expire;
								s.players = 0;

								v[i]->servers.push_back(s);

								printf("ADMIN_REQ_OK >> %s : adds - %s\n", conn->remote_ip, srvname);
								masterLog.LineOut(true, "ADMIN_REQ_OK >> %s : adds - %s\n", conn->remote_ip, srvname);
								mg_printf_data(conn, "OK!\nC_REF: %s\nServer Name: %s\nServer IP: %s\nExpire: %s\n\nAdd To >> %s", c_ref, srvname, srvip, expire, email);
							}
						}
					}
					else
					{
						printf("ADMIN_REQ_ERROR >> %s : %s%s\n", conn->remote_ip, conn->uri, conn->query_string);
						masterLog.LineOut(true, "ADMIN_REQ_ERROR >> %s : %s%s", conn->remote_ip, conn->uri, conn->query_string);
					}
				}
				else
				{
					printf("REQ_ERROR >> %s : %s%s\n", conn->remote_ip, conn->uri, conn->query_string);
					masterLog.LineOut(true, "REQ_ERROR >> %s : %s%s", conn->remote_ip, conn->uri, conn->query_string);
				}
			}
			else
			{
				printf("REQ_ERROR >> %s : %s%s\n", conn->remote_ip, conn->uri, conn->query_string);
				masterLog.LineOut(true, "REQ_ERROR >> %s : %s%s", conn->remote_ip, conn->uri, conn->query_string);
			}
		}

		return MG_TRUE;
	}
	else if (ev == MG_AUTH)
	{
		return MG_TRUE;
	}
	else
		return MG_FALSE;
}

void main()
{
	ghp_UTILS::DataChunk chunk;
	ghp_UTILS::LoadFileData(L"db.bin", &chunk);

	db.open("db.bin");
	db.registerBeanClass<Server>();
	db.registerBeanClass<Cliente>();

	if (chunk.size == 0)
	{
		masterLog.LineOut(false, "[WARNING] db.bin not found! Creating a new one!");
		db.dropModel();
		db.createModel();
	}
	
	SetConsoleTitleA("[GHP] MasterServer");

	struct mg_server * server;
	
	server = mg_create_server(NULL, server_request);
	mg_set_option(server, "listening_port", "55506");
		
	while (true)
	{
		mg_poll_server(server, 1000);
	}

	mg_destroy_server(&server);
}