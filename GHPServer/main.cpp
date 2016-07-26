#include "..\include.h"
#include "..\ghp_NET\mongoose.h"
#include "ServerCore.h"

#include "..\ghp_UTILS\tinyxml2\tinyxml2.h"
#include "..\ghp_UTILS\ghp_PROTOCOL.h"
#include "..\ghp_UTILS\ghp_LOG.h"
#include "..\ghp_CRYPTO\ghp_CRYPTO.h"
#include "..\ghp_NET\ghp_HTTPCLIENT.h"
#include "..\ghp_UTILS\ghp_SYSTEM.h"
#include "..\ghp_UTILS\ghp_STRUCTURED_FILE.h"

#include <Pdh.h>
#pragma comment (lib, "Pdh.lib")

PDH_HQUERY cpuQuery;
PDH_HCOUNTER cpuTotal;

int BIND_GHPSERVER;
std::string BIND_WEBSERVER = "";
std::string AUTH_SYSTEM = "";
std::string AUTH_USER = "";
std::string AUTH_PASSWORD = "";
std::string STATUS_NOTIFY = "";
std::string ACC_BAN_URL = "";

bool acc_autoban;
bool hwid_autoban;
bool ip_autoban;
bool call_api;
bool force_api;

int max_connections, ss_quality, ss_grayscale, auto_ss_interval;

std::string V_DOWNLOAD_DLL	= "dll";
std::string V_DOWNLOAD_LIC	= "lic";
std::string V_DOWNLOAD_DB	= "db";
std::string V_DOWNLOAD_MF	= "mf";
std::string V_DOWNLOAD_CWL	= "cwl";
std::string V_DOWNLOAD_AWL	= "awl";

std::string V_INFO			= "info";
std::string V_PLAYERS_ON	= "players_on";
std::string V_ALL_PLAYERS	= "all_players";
std::string V_CONFIGS		= "configs";
std::string V_VIEW			= "view";

std::string A_BANHWID	 = "1";
std::string A_BANUSERID  = "2";
std::string A_BANIP		 = "3";
std::string A_REQSS		 = "4";
std::string A_DISCONNECT = "5";
std::string A_DEL_LOGS   = "6";
std::string A_DEL_SS	 = "7";

std::string DLL_HASH, LIC_HASH, DB_HASH;

NOTIFYICONDATA NID;
ghp_UTILS::DataChunk PUB_KEY, PRV_KEY, GHP_DLL, GHP_DB, GHP_LIC;
std::string C_REF = "";
std::vector<std::string> C_DATA;

#define MASTERSERVER_LINK		"http://sv.gamehackprotector.com:55506/?c_ref="

#define WEB_REDIRECT			"<!DOCTYPE html> <html> <head> <meta http-equiv=\"Refresh\" content=\"0; url=%s\"> </head> <body> </body> </html>"
#define WEB_LOGIN_PAGE			"<!DOCTYPE html> <html> <head> <title>%ls</title> <style> * { font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif; font-size: 14px; } .text { width: 230px; height: 26px; border: #CCC solid 1px; margin-bottom: 10px; padding-left: 6px; } .text:focus { outline: 0; border: #1B8DB7 solid 1px; } .button { width: 90px; height: 28px; color: #FFF; background-color: #1B8DB7; border: 0; } .button:hover { outline: 0; background-color: #00668B; } .button:focus { outline: 0; background-color: #000; } .tip { color: #666; position: absolute; top: 7px; left: 8px; } .hidden { display: none; } #loginbox { background-color: #F3F3F3; width: 240px; margin: 180px auto 0px auto; padding: 20px; border: #DEDEDE solid 1px; } </style> </head>  <body> <div id=\"loginbox\"> <form action=\"#\" method=\"post\"> <div style=\"position: relative;\"> <input id=\"textlogin\" class=\"text\" type=\"text\" name=\"login\" maxlength=\"16\" value=\"Login\" onclick=\"if(this.value=='Login') this.value='';\" onblur=\"if(this.value=='') this.value='Login';\"> </div> <div style=\"position: relative;\"> <input id=\"textpassword\" class=\"text\" type=\"password\" name=\"password\" maxlength=\"20\" value=\"Senha\" onclick=\"if(this.value=='Senha') this.value='';\" onblur=\"if(this.value=='') this.value='Senha';\"> </div> <input class=\"button\" type=\"submit\" value=\"%s\"> </form> </div> </body> </html>"

#define HTML_HEADER				"<!DOCTYPE html> <html> <head> <title>[GHPSrv] %s</title> <style> * { margin: 0; padding: 0; border: 0; cursor: default; -moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none; font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif; font-size: 14px; list-style: none; } #header { color: #FFF; background-color: #1B8DB7; height: 60px; } #body { position: absolute; left: 0; top: 60px; right: 0; bottom: 24px; } #footer { position: absolute; left: 0; bottom: 0; width: 100%%; height: 24px; background-color: #1B8DB7; } #footer > #ccu { float: left; color: #FFF; } #footer > #ccu p { float: left; font-size: 13px; margin: 4px 8px; } #footer > #info { float: right; color: #FFF; } #footer > #info p { float: right; font-size: 13px; margin: 4px 8px; } #menu { position: absolute; left: 0; top: 0; bottom: 0; width: 200px; background-color: #F3F3F3; } #menu ul { margin-top: 8px; } #menu li { font-size: 14px; padding: 9px 13px; cursor: pointer; } #menu li:hover { background-color: #E7E7E7; } #menu li.selected { background-color: #D4D4D4; } #players { position: absolute; left: 200px; top: 0; bottom: 0; right: 0; overflow: auto; } #players ul { margin: 8px 0px; } #players li { padding: 9px 16px; cursor: pointer; } #players li:hover { background-color: #E7E7E7; } #view { position: absolute; left: 200px; top: 0; bottom: 0; right: 0; overflow: auto; } #view ul { margin: 8px 0px; } #view li	{ padding: 9px 16px; } span { display: inline-block; width: 180px; cursor: pointer; } span.litle { width: 130px; } span.big { width: 260px; } #logo { float: left; margin: 9px 16px;  } #logo p.big { float: left; font-size: 36px; font-weight: bold; } #logo p.medium { float: left; font-size: 16px; font-weight: bold; position: relative; top: 13px; left: 10px; } #serverinfo { float: right; } #serverinfo p.name { float: right; font-size: 20px; font-weight: bold; margin: 20px; } .botao { -moz-box-shadow:inset 0px 0px 0px 0px #ffffff; -webkit-box-shadow:inset 0px 0px 0px 0px #ffffff; box-shadow:inset 0px 0px 0px 0px #ffffff; background:-webkit-gradient( linear, left top, left bottom, color-stop(0.05, #ededed), color-stop(1, #dfdfdf) ); background:-moz-linear-gradient( center top, #ededed 5%%, #dfdfdf 100%% ); filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ededed', endColorstr='#dfdfdf'); background-color:#ededed; -webkit-border-top-left-radius:15px; -moz-border-radius-topleft:15px; border-top-left-radius:15px; -webkit-border-top-right-radius:15px; -moz-border-radius-topright:15px; border-top-right-radius:15px; -webkit-border-bottom-right-radius:15px; -moz-border-radius-bottomright:15px; border-bottom-right-radius:15px; -webkit-border-bottom-left-radius:15px; -moz-border-radius-bottomleft:15px; border-bottom-left-radius:15px; text-indent:0px; display:inline-block; color:#777777; font-family:arial; font-size:14px; font-weight:bold; font-style:normal; height:27px; line-height:27px; width:152px; text-decoration:none; text-align:center; text-shadow:1px 1px 0px #ffffff; outline: none; } .botao:hover { background:-webkit-gradient( linear, left top, left bottom, color-stop(0.05, #dfdfdf), color-stop(1, #ededed) ); background:-moz-linear-gradient( center top, #dfdfdf 5%%, #ededed 100%% ); filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#dfdfdf', endColorstr='#ededed'); background-color:#dfdfdf; cursor: pointer; } .botao:active { position:relative; top:1px; } .clean { display: inline; position: relative; text-decoration:none; color: #1B8DB7; } .clean:hover { cursor:pointer; color: #136382; } .tooltip { display: inline; position: relative; text-decoration:none; } .tooltip:hover:after { background: #1B8DB7; background: rgb(27, 141, 183); border-radius: 5px; top: 30px; color: #fff; content: attr(title); left: 0; padding: 5px 15px; position: absolute; z-index: 98; width: 220px; } .tooltip:hover:before { border: solid; border-color: #1B8DB7 transparent; border-width: 0px 6px 6px 6px; top: 25px; content: \"\"; left: 50%%; position: absolute; z-index: 99; } .ghp_info, .ghp_success, .ghp_warning, .ghp_error { border: 1px solid; margin: 0px 0px; padding:20px 10px 15px 50px; background-repeat: no-repeat; background-position: 10px center;-moz-border-radius:.5em; -webkit-border-radius:.5em; border-radius:.0em; display: inline; position: relative; font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif; font-size: 20px; } .ghp_info { color: #00529B; background-color: #BDE5F8; background-image: url('http://i.imgur.com/zqxmcZX.png'); } .ghp_success { color: #4F8A10; background-color: #DFF2BF; background-image:url('http://i.imgur.com/ic8Vim4.png'); } .ghp_warning { color: #9F6000; background-color: #FEEFB3; background-image: url('http://i.imgur.com/G4Ok9va.png'); } .ghp_error { color: #D8000C; background-color: #FFBABA; background-image: url('http://i.imgur.com/f9f3L96.png'); } #message { position: fixed; top: 0; left: 0; right: 0; z-index: 100; } </style> <script> var xmlhttp; UpdateCCU(); setInterval(UpdateCCU, 1000); function UpdateCCU() { if (window.XMLHttpRequest) { xmlhttp=new XMLHttpRequest(); } else { xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\"); } if (xmlhttp == null) return; xmlhttp.open(\"GET\",\"?v=info\",true); xmlhttp.onreadystatechange = CCUChange; xmlhttp.send(null); } function CCUChange() { if (xmlhttp.readyState == 4 && xmlhttp.status == 200) { document.getElementById(\"ccu\").innerHTML = xmlhttp.responseText; } else { document.getElementById(\"ccu\").innerHTML = \"<p>GHPServer OFFLINE</p>\"; } } function OpenLink(link) { window.open(link, '_self'); } </script> </head> <body%s> <div id=\"header\"> <div id=\"logo\"> <p class=\"big\">GHP</p> <p class=\"medium\">Game Hack Protector</p> </div> <div id=\"serverinfo\"> <p class=\"name\">%s</p> </div> </div> <div id=\"body\">"
#define HTML_MENU_PLAYERS_ON	"<div id=\"menu\"> <ul> <li class=\"selected\" onclick=\"OpenLink('?v=players_on')\">%s</li> <li onclick=\"OpenLink('?v=all_players')\">%s</li> <li onclick=\"OpenLink('?v=configs')\">%s</li> </ul> </div>"
#define HTML_MENU_ALL_PLAYERS	"<div id=\"menu\"> <ul> <li onclick=\"OpenLink('?v=players_on')\">%s</li> <li class=\"selected\" onclick=\"OpenLink('?v=all_players')\">%s</li> <li onclick=\"OpenLink('?v=configs')\">%s</li> </ul> </div>"
#define HTML_MENU_CONFIGS		"<div id=\"menu\"> <ul> <li onclick=\"OpenLink('?v=players_on')\">%s</li> <li onclick=\"OpenLink('?v=all_players')\">%s</li> <li class=\"selected\" onclick=\"OpenLink('?v=configs')\">%s</li> </ul> </div>"
#define HTML_FOOTER				"</div> <div id=\"footer\"> <div id=\"ccu\"></div> <div id=\"info\"><p>%s %s</p></div> </div> </body> </html>"

std::string LOGIN_BUTTON_LNG;
std::string PLAYERS_ON_LNG, ALL_PLAYERS_LNG, OPTIONS_LNG;
std::string REMAINING_DAYS_LNG;

std::string NO_LONGER_ONLINE_LNG;
std::string BLCK_BY_ADMIN_REASON_LNG, BLCK_HACK_USE_LNG;
std::string HWID_BAN_ERROR_LNG, HWID_BAN_OK_LNG, USER_BAN_ERROR_LNG, USER_BAN_OK_LNG, IP_BAN_OK_LNG, UNEXPECTED_ERROR_LNG, DISABLED_FUNCTION_LNG, PLAYER_DISCONNECTED_LNG, SCREENSHOT_REQUEST_OK_LNG, SCREENSHOT_REQUEST_ERROR_LNG;
std::string PLAYER_INFO_LNG, PLAYER_OP_LNG, PLAYER_LOGS_LNG, PLAYER_SS_LNG;
std::string HWID_BAN_HINT_LNG, USER_BAN_HINT_LNG, IP_BAN_HINT_LNG, SS_HINT_LNG, DC_HINT_LNG;
std::string HWID_BAN_BT_LNG, USER_BAN_BT_LNG, IP_BAN_BT_LNG, SS_BT_LNG, DC_BT_LNG;
std::string REFRESH_BT_LNG, REFRESH_HINT_LNG, DELETE_LOGS_LNG, DELETE_SS_LNG;
std::string NEVER_PLAYED_LNG, BEFORE_LNG, MINUTE_LNG, HOUR_LNG, DAY_LNG;

#define CALLBACK_MESSAGE		WM_USER + 1
#define ID_MENU_EXIT			WM_USER + 3

ghp_UTILS::Log srvLog(L"GHPServer.log", true);

extern void SendPacket(UCHAR * key, RakNet::AddressOrGUID addr, LPVOID Packet, UINT size, bool sendNow = false);

std::string ExtractUserIDFromHWIDPath(std::wstring path)
{
	std::string ret(path.begin(), path.end());

	ret.erase(ret.begin(), ret.begin() + 12);
	ret.erase(ret.end() - 4, ret.end());

	return ret;
}

std::string GetLastLoginTime(std::string userid)
{
	std::string path = ".\\data\\info\\" + userid + ".ini";

	char lastlogin[64];
	GetPrivateProfileStringA("info", "lastlogin", "0", lastlogin, 64, path.c_str());

	time_t now;
	time_t raw = atol(lastlogin);
	struct tm local;
	localtime_s(&local, &raw);

	// ùltimo login não encontrado
	if (raw == 0)
		return NEVER_PLAYED_LNG;

	time(&now);
	double timediff = difftime(now, raw);
	double finaldiff = floor(timediff / 60.0f);
	std::string info;

	if (finaldiff == 0.0f)
		finaldiff = 1.0f;

	if (finaldiff == 1.0f) info = MINUTE_LNG + " " + BEFORE_LNG;
	else info = MINUTE_LNG + "s " + BEFORE_LNG;

	// horas
	if (finaldiff >= 60.0f)
	{
		finaldiff /= 60.0f;

		if (finaldiff == 1.0f) info = HOUR_LNG + " " + BEFORE_LNG;
		else info = HOUR_LNG + "s " + BEFORE_LNG;

		// dias
		if (finaldiff >= 24.0f)
		{
			finaldiff /= 24.0f;
			
			if (finaldiff == 1.0f) info = DAY_LNG + " " + BEFORE_LNG;
			else info = DAY_LNG + "s " + BEFORE_LNG;
		}
	}
	
	char ret[128];
	sprintf_s(ret, "%02d/%02d/%02d %02d:%02d (%.f %s)", local.tm_mday, local.tm_mon, local.tm_year - 100, local.tm_hour, local.tm_min, finaldiff, info.c_str());
	return ret;
}

void WEB_Redirect(struct mg_connection * conn, std::string URL)
{
	mg_printf_data(conn, VMProtectDecryptStringA(WEB_REDIRECT), URL.c_str());
}

void WEB_LoginPage(struct mg_connection * conn)
{
	mg_printf_data(conn, VMProtectDecryptStringA(WEB_LOGIN_PAGE), VMProtectDecryptStringW(GHP_SERVER_TITLE), LOGIN_BUTTON_LNG.c_str());
}

void WEB_ServerPage(struct mg_connection * conn)
{
	std::string onLoad = "";
	std::string actionString = "";
	std::string classString = "";

	Player player;
	char v[32];
	char guid[64];
	char playerstr[64];
	char action[10];
	bool onlinePlayer = false;

	mg_get_var(conn, "v", v, 32);
	mg_get_var(conn, "guid", guid, 64);
	mg_get_var(conn, "player", playerstr, 64);
	mg_get_var(conn, "action", action, 10);

	if (strlen(guid) > 1)
		onlinePlayer = true;
	else
	{
		// Define os valores para funcionar posteriormente na lista offline
		player.userid = playerstr;
		player.ip = "no data";
	}

	// Pega o player com base no GUID
	if (V_VIEW == v && onlinePlayer)
	{
		player = GetPlayerByGUID(guid);

		if (player.guid.systemIndex == RakNet::UNASSIGNED_PLAYER_INDEX)
		{
			actionString = NO_LONGER_ONLINE_LNG;
			classString = "ghp_info";
		}
	}

	// Se for INFO, retorna
	if (V_INFO == v)
	{
		PDH_FMT_COUNTERVALUE counterVal;
		PdhCollectQueryData(cpuQuery);
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);

		float mem = (((float)memInfo.ullTotalPhys - (float)memInfo.ullAvailPhys) / (float)memInfo.ullTotalPhys) * 100.0f;

		mg_printf_data(conn, VMProtectDecryptStringA("<p>%d online | CPU: %.f%% - RAM: %.f%%</p>"), players.size(), counterVal.doubleValue, mem);
		return;
	}

	mg_printf_data(conn, VMProtectDecryptStringA(HTML_HEADER), C_DATA[0].c_str(), " onload=\"setTimeout(function () { document.getElementById('message').style.visibility = 'hidden'; } , 3000);\"", C_DATA[0].c_str());

	if (V_CONFIGS == v)
		mg_printf_data(conn, VMProtectDecryptStringA(HTML_MENU_CONFIGS), PLAYERS_ON_LNG.c_str(), ALL_PLAYERS_LNG.c_str(), OPTIONS_LNG.c_str());
	else if ((onlinePlayer && V_VIEW == v && player.guid.systemIndex != RakNet::UNASSIGNED_PLAYER_INDEX) || (!onlinePlayer && V_VIEW == v))
	{
		BanResult ret = BANR_ERROR;

		// Verifica as ações
		if (A_BANHWID == action)
		{
			ret = BANR_OK;
			std::vector<std::string> hwids = GetHWIDSFromFile(player.userid);

			for (UINT i = 0; i < hwids.size(); i++)
			{
				if (BanData(BAN_HWID, hwids[i], BLCK_BY_ADMIN_REASON_LNG, player.ip, player.userid) != BANR_OK)
					ret = BANR_ERROR;
			}

			if (hwids.size() == 0)
				ret = BANR_ERROR;

			if (ret == BANR_OK)
			{
				actionString = HWID_BAN_OK_LNG;
				classString = "ghp_success";
			}
			else
			{
				actionString = HWID_BAN_ERROR_LNG;
				classString = "ghp_warning";
			}

			// Desconecta o player
			if (onlinePlayer)
				peer->CloseConnection(player.guid, true);
		}
		else if (A_BANUSERID == action)
		{
			if (player.userid != "")
			{
				ret = BanData(BAN_USERID, player.userid, BLCK_BY_ADMIN_REASON_LNG, player.ip, player.userid);
			
				if (ret == BANR_OK)
				{
					actionString = USER_BAN_OK_LNG;
					classString = "ghp_success";

					// Desconecta o player
					if (onlinePlayer)
						peer->CloseConnection(player.guid, true);
				}
				else
				{
					actionString = UNEXPECTED_ERROR_LNG;
					classString = "ghp_warning";
				}
			}
			else
			{
				actionString = USER_BAN_ERROR_LNG;
				classString = "ghp_error";
			}
		}
		else if (A_BANIP == action)
		{
			ret = BanData(BAN_IP, player.ip, BLCK_BY_ADMIN_REASON_LNG, player.ip, player.userid);
			
			if (ret == BANR_OK)
			{
				actionString = IP_BAN_OK_LNG;
				classString = "ghp_success";

				// Desconecta o player
				if (onlinePlayer)
					peer->CloseConnection(player.guid, true);
			}
			else
			{
				actionString = UNEXPECTED_ERROR_LNG;
				classString = "ghp_warning";
			}

		}
		else if (A_REQSS == action)
		{
			if (timeGetTime() - player.lastSS > 15000)
			{
				actionString = SCREENSHOT_REQUEST_OK_LNG;
				classString = "ghp_success";

				SetPlayerLastSSByGUID(player.guid);

				PCK_SS packet_ss(PCK_SS_H_REQUEST, (UCHAR)ss_quality, (UCHAR)ss_grayscale, NULL);
				SendPacket(player.key, player.guid, &packet_ss, packet_ss.buffer_size + 8);
			}
			else
			{
				actionString = SCREENSHOT_REQUEST_ERROR_LNG;
				classString = "ghp_warning";
			}			
		}
		else if (A_DISCONNECT == action)
		{
			actionString = PLAYER_DISCONNECTED_LNG;
			classString = "ghp_success";
			peer->CloseConnection(player.guid, true);
		}
		else if (A_DEL_LOGS == action)
		{
			std::wstring path = L".\\data\\logs\\login\\" + std::wstring(player.userid.begin(), player.userid.end()) + L".log";
			DeleteFileW(path.c_str());
		}
		else if (A_DEL_SS == action)
		{
			std::wstring wpath = L".\\data\\ss\\" + std::wstring(player.userid.begin(), player.userid.end()) + L"\\";
			std::vector<std::wstring> ss;
			ss.clear();
			ghp_UTILS::GetAllFiles(ss, wpath);

			for (UINT i = 0; i < ss.size(); i++)
			{
				DeleteFileW(ss[i].c_str());
			}
		}

		// Calcula o tempo que o player está online
		DWORD currentTime = timeGetTime();
		DWORD playerTimeH = 0, playerTimeM = 0;

		playerTimeM = ((currentTime - player.connectionTime) / 1000) / 60;
		
		if (playerTimeM >= 60)
		{
			playerTimeH = playerTimeM / 60;
			playerTimeM = playerTimeM % (playerTimeH * 60);
		}
		else
			playerTimeH = 0;
		// ---------------------------------------------------

		mg_printf_data(conn, VMProtectDecryptStringA(onlinePlayer ? HTML_MENU_PLAYERS_ON : HTML_MENU_ALL_PLAYERS), PLAYERS_ON_LNG.c_str(), ALL_PLAYERS_LNG.c_str(), OPTIONS_LNG.c_str());
		mg_printf_data(conn, "<div id=\"view\"> <ul>");

		mg_printf_data(conn, "<li style=\"background-color:transparent;\"><b>%s</b></li>", PLAYER_INFO_LNG.c_str());
		if (onlinePlayer)
			mg_printf_data(conn, "<li style=\"cursor: default;\"><span class=\"litle\" style=\"cursor: default;\">%s</span><span class=\"litle\" style=\"cursor: default;\">%s</span><span class=\"litle\" style=\"cursor: default;\">%d ms</span><span class=\"litle\" style=\"cursor: default;\">%dh %dm</span><span style=\"cursor: default;\">%s</span></li>", player.ip.c_str(), player.userid.c_str(), player.connectionPing, playerTimeH, playerTimeM, player.guid.ToString());
		else
			mg_printf_data(conn, "<li style=\"cursor: default;\"><span class=\"litle\" style=\"cursor: default;\">%s</span><span class=\"big\" style=\"cursor: default;\">%s</span></li>", player.userid.c_str(), GetLastLoginTime(player.userid).c_str());

		mg_printf_data(conn, "<li style=\"background-color:transparent;\"><b>%s</b></li>", PLAYER_OP_LNG.c_str());

		if (onlinePlayer)
			mg_printf_data(conn, "<li style=\"background-color:transparent;\"> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" class=\"botao\" title=\" \" name=\"banirhwid\" value=\"%s\" onClick=\"OpenLink('?v=view&guid=%s&action=1')\"> </a> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" name=\"banirlogin\" class=\"botao\" title=\" \" value=\"%s\" onClick=\"OpenLink('?v=view&guid=%s&action=2')\"> </a> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" name=\"banirip\" class=\"botao\" title=\" \" value=\"%s\" onClick=\"OpenLink('?v=view&guid=%s&action=3')\"> </a> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" name=\"screenshot\" class=\"botao\" title=\" \" value=\"%s\" onClick=\"OpenLink('?v=view&guid=%s&action=4')\"> </a> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" name=\"desconectar\" class=\"botao\" title=\" \" value=\"%s\" onClick=\"OpenLink('?v=view&guid=%s&action=5')\"> </a> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" name=\"refresh\" class=\"botao\" title=\" \" value=\"%s\" onClick=\"OpenLink('?v=view&guid=%s')\"> </a> </li>", HWID_BAN_HINT_LNG.c_str(), HWID_BAN_BT_LNG.c_str(), guid, USER_BAN_HINT_LNG.c_str(), USER_BAN_BT_LNG.c_str(), guid, IP_BAN_HINT_LNG.c_str(), IP_BAN_BT_LNG.c_str(), guid, SS_HINT_LNG.c_str(), SS_BT_LNG.c_str(), guid, DC_HINT_LNG.c_str(), DC_BT_LNG.c_str(), guid, REFRESH_HINT_LNG.c_str(), REFRESH_BT_LNG.c_str(), guid);
		else
			mg_printf_data(conn, "<li style=\"background-color:transparent;\"> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" class=\"botao\" title=\" \" name=\"banirhwid\" value=\"%s\" onClick=\"OpenLink('?v=view&player=%s&action=1')\"> </a> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" name=\"banirlogin\" class=\"botao\" title=\" \" value=\"%s\" onClick=\"OpenLink('?v=view&player=%s&action=2')\"> </a> <a href=\"javascript:void(0);\" class=\"tooltip\" title=\"%s\"> <input type=\"button\" name=\"refresh\" class=\"botao\" title=\" \" value=\"%s\" onClick=\"OpenLink('?v=view&player=%s')\"> </a> </li>", HWID_BAN_HINT_LNG.c_str(), HWID_BAN_BT_LNG.c_str(), playerstr, USER_BAN_HINT_LNG.c_str(), USER_BAN_BT_LNG.c_str(), playerstr, REFRESH_HINT_LNG.c_str(), REFRESH_BT_LNG.c_str(), playerstr);

		if (classString != "" && actionString != "")
			mg_printf_data(conn, "<div id=\"message\" class=\"%s\">%s</div>", classString.c_str(), actionString.c_str());

		// Logs
		std::wstring path = L".\\data\\logs\\login\\" + std::wstring(player.userid.begin(), player.userid.end()) + L".log";
		ghp_UTILS::DataChunk log;
		std::string logs = "";

		if (!ghp_UTILS::LoadFileData(path.c_str(), &log))
			log.FreeAll();
		else
			logs = log.toString();		

		if (onlinePlayer)
			mg_printf_data(conn, "<li style=\"background-color:transparent;\"><b>%s</b> - <a href=\"javascript:void(0);\" class=\"clean\" onClick=\"OpenLink('?v=view&guid=%s&action=6')\">%s</a> </li>", PLAYER_LOGS_LNG.c_str(), guid, DELETE_LOGS_LNG.c_str());
		else
			mg_printf_data(conn, "<li style=\"background-color:transparent;\"><b>%s</b> - <a href=\"javascript:void(0);\" class=\"clean\" onClick=\"OpenLink('?v=view&player=%s&action=6')\">%s</a> </li>", PLAYER_LOGS_LNG.c_str(), playerstr, DELETE_LOGS_LNG.c_str());

		mg_printf_data(conn, "<li style=\"white-space: pre; -moz-user-select: auto;-webkit-user-select: auto;-ms-user-select:auto;	user-select:auto; cursor: text; \">%s</li>", logs.c_str());
		log.FreeAll();

		// SS
		std::wstring wpath = L".\\data\\ss\\" + std::wstring(player.userid.begin(), player.userid.end()) + L"\\";
		std::vector<std::wstring> ss;
		ss.clear();
		ghp_UTILS::GetAllFiles(ss, wpath);

		if (onlinePlayer)
			mg_printf_data(conn, "<li style=\"background-color:transparent;\"><b>%s</b> - <a href=\"javascript:void(0);\" class=\"clean\" onClick=\"OpenLink('?v=view&guid=%s&action=7')\">%s</a></li><li style=\"background-color:transparent;\">", PLAYER_SS_LNG.c_str(), guid, DELETE_SS_LNG.c_str());
		else
			mg_printf_data(conn, "<li style=\"background-color:transparent;\"><b>%s</b> - <a href=\"javascript:void(0);\" class=\"clean\" onClick=\"OpenLink('?v=view&player=%s&action=7')\">%s</a></li><li style=\"background-color:transparent;\">", PLAYER_SS_LNG.c_str(), playerstr, DELETE_SS_LNG.c_str());
		
		for (UINT i = 0; i < ss.size(); i++)
		{
			std::string temp = std::string(ss[i].begin(), ss[i].end());
			temp.erase(temp.begin(), temp.begin() + 1);
			std::replace(temp.begin(), temp.end(), '\\', '/');

			std::string URL = "<a href=\"" + temp + "\" target=\"_blank\"><img style=\"padding-top: 4px; padding-right: 8px; cursor: pointer;\" src=\"" + temp + "\" title=\"...\" width=\"240px\"></a>";
			mg_printf_data(conn, URL.c_str());
		}

		mg_printf_data(conn, "</li></ul> </div>");
	}
	else if (V_ALL_PLAYERS == v)
	{
		// Lista todos os players (HWID list)
		std::vector<std::wstring> all_players;
		all_players.clear();
		ghp_UTILS::GetAllFiles(all_players, L".\\data\\hwid\\");

		mg_printf_data(conn, VMProtectDecryptStringA(HTML_MENU_ALL_PLAYERS), PLAYERS_ON_LNG.c_str(), ALL_PLAYERS_LNG.c_str(), OPTIONS_LNG.c_str());
		mg_printf_data(conn, "<div id=\"players\"> <ul>");

		for (UINT i = 0; i < all_players.size(); i++)
		{
			std::string player = ExtractUserIDFromHWIDPath(all_players[i]);
			Player p = GetPlayerByUserID(player);

			if (p.userid == player)
				mg_printf_data(conn, "<li onClick=\"OpenLink('?v=view&guid=%s')\"><span>%s</span><span><b style=\"color: #009900;\">%s</b></span></li>", p.guid.ToString(), player.c_str(), "ONLINE");
			else
				mg_printf_data(conn, "<li onClick=\"OpenLink('?v=view&player=%s')\"><span>%s</span><span><b style=\"color: #FF0000;\">%s</b></span></li>", player.c_str(), player.c_str(), "OFFLINE");
		}

		mg_printf_data(conn, "</ul> </div>");
	}
	else
	{
		// Calcula o tempo que o player está online
		DWORD currentTime = timeGetTime();
		DWORD playerTimeH = 0, playerTimeM = 0;

		mg_printf_data(conn, VMProtectDecryptStringA(HTML_MENU_PLAYERS_ON), PLAYERS_ON_LNG.c_str(), ALL_PLAYERS_LNG.c_str(), OPTIONS_LNG.c_str());
		mg_printf_data(conn, "<div id=\"players\"> <ul>");

		if (classString != "" && actionString != "")
			mg_printf_data(conn, "<div id=\"message\" class=\"%s\">%s</div>", classString.c_str(), actionString.c_str());

		for (UINT i = 0; i < players.size(); i++)
		{
			playerTimeM = ((currentTime - players[i].connectionTime) / 1000) / 60;

			if (playerTimeM >= 60)
			{
				playerTimeH = playerTimeM / 60;
				playerTimeM = playerTimeM % (playerTimeH * 60);
			}
			else
				playerTimeH = 0;

			mg_printf_data(conn, "<li onClick=\"OpenLink('?v=view&guid=%s')\"><span class=\"litle\">%s</span><span class=\"litle\">%s</span><span class=\"litle\">%d ms</span><span class=\"litle\">%dh %dm</span><span>%s</span></li>", players[i].guid.ToString(), players[i].ip.c_str(), players[i].userid.c_str(), players[i].connectionPing, playerTimeH, playerTimeM, players[i].guid.ToString());
		}
		mg_printf_data(conn, "</ul> </div>");
	}

	mg_printf_data(conn, VMProtectDecryptStringA(HTML_FOOTER), C_DATA[4].c_str(), REMAINING_DAYS_LNG.c_str());
}

void WEB_SendFile(struct mg_connection * conn, std::wstring fileName)
{
	ghp_UTILS::DataChunk file;
	if (ghp_UTILS::LoadFileData(fileName, &file))
	{
		mg_send_data(conn, file.data, file.size);
	}
}

int server_request(struct mg_connection * conn, enum mg_event ev)
{
	if (ev == MG_REQUEST)
	{
		// Verifica se é player_status (checagem de players online ou não)
		char pl_status[32];
		memset(pl_status, 0, 32);
		mg_get_var(conn, "pl_status", pl_status, 32);

		if (strlen(pl_status) > 1)
		{
			char pl_status_key[64];
			memset(pl_status_key, 0, 64);
			mg_get_var(conn, "key", pl_status_key, 64);

			// Verifica a key
			if (C_REF != pl_status_key)
			{
				mg_printf_data(conn, "0");
				return MG_TRUE;
			}

			// Pega o player
			Player player = GetPlayerByUserID(pl_status);

			// Retorna 0 ou 1, significando logado ou não logado
			if (player.userid == pl_status)
				mg_printf_data(conn, "1");
			else
				mg_printf_data(conn, "0");

			return MG_TRUE;
		}
		
		// Verifica se é alguma SS e caso seja retorna
		std::string url = conn->uri;
		if (url.size() > 10)
		{
			url.erase(url.begin() + 9, url.end());
			if (url == VMProtectDecryptStringA("/data/ss/"))
			{
				url = conn->uri;
				std::replace(url.begin(), url.end(), '/', '\\');
				url = "." + url;

				std::wstring wurl(url.begin(), url.end());
				WEB_SendFile(conn, wurl);
				return MG_TRUE;
			}
		}

		// Verifica se é download e caso seja, baixa e retorna
		char download[16];
		mg_get_var(conn, "download", download, 16);
		if (V_DOWNLOAD_DLL == download)
		{
			WEB_SendFile(conn, L".\\data\\update\\DLL");
			return MG_TRUE;
		}
		else if (V_DOWNLOAD_DB == download)
		{
			WEB_SendFile(conn, L".\\data\\update\\DB");
			return MG_TRUE;
		}
		else if (V_DOWNLOAD_LIC == download)
		{
			WEB_SendFile(conn, L".\\data\\update\\LIC");
			return MG_TRUE;
		}
		else if (V_DOWNLOAD_MF == download)
		{
			// Cria o XML
			tinyxml2::XMLDocument doc;
			tinyxml2::XMLNode * updateInfo = doc.InsertFirstChild(doc.NewElement("UpdateInfo"));

			// Captura os HASH e monta o XML (somente se os arquivos estiverem presente)
			if (GHP_DLL.size != 0 && GHP_DB.size != 0 && GHP_LIC.size != 0)
			{
				tinyxml2::XMLElement * DLL = doc.NewElement("DLL"); DLL->SetText(DLL_HASH.c_str()); updateInfo->InsertFirstChild(DLL);
				tinyxml2::XMLElement * DB  = doc.NewElement("DB");  DB->SetText(DB_HASH.c_str());   updateInfo->InsertAfterChild(DLL, DB);
				tinyxml2::XMLElement * LIC = doc.NewElement("LIC"); LIC->SetText(LIC_HASH.c_str()); updateInfo->InsertAfterChild(DB, LIC);
			}
			
			// Retorna o XML
			tinyxml2::XMLPrinter printer(0, true, 0);
			doc.Accept(&printer);
			mg_printf_data(conn, printer.CStr());
			doc.Clear();
			return MG_TRUE;
		}
		else if (V_DOWNLOAD_CWL == download)
		{
			ghp_UTILS::DataChunk chunk;

			// Se o arquivo existir, envia-o. Caso contrário, envia 0 para identificar que não existe whitelist
			if (ghp_UTILS::LoadFileData(L"CheatWhitelist.bin", &chunk))
				WEB_SendFile(conn, L".\\CheatWhitelist.bin");
			else
				mg_printf_data(conn, "0");

			chunk.FreeAll();
			return MG_TRUE;
		}
		else if (V_DOWNLOAD_AWL == download)
		{
			ghp_UTILS::DataChunk chunk;

			// Se o arquivo existir, envia-o. Caso contrário, envia 0 para identificar que não existe whitelist
			if (ghp_UTILS::LoadFileData(L"APIWhitelist.bin", &chunk))
				WEB_SendFile(conn, L".\\APIWhitelist.bin");
			else
				mg_printf_data(conn, "0");

			chunk.FreeAll();
			return MG_TRUE;
		}

		// Verifica se a aquisição vem do usuário logado
		if (AUTH_SYSTEM == conn->remote_ip)
		{
			WEB_ServerPage(conn);
		}
		else
		{
			char user[32];
			char pass[32];

			// Pega as variáveis do post
			mg_get_var(conn, "login", user, 32);
			mg_get_var(conn, "password", pass, 32);

			// Verifica as credenciais
			if (AUTH_USER == user && AUTH_PASSWORD == pass)
			{
				// Salva no log
				srvLog.LineOut(true, "GHPServer WEBUI conection from %s", conn->remote_ip);
				// Autoriza o usuário
				AUTH_SYSTEM = conn->remote_ip;
				// Mostra a página inicial do GHPServer
				WEB_ServerPage(conn);
			}
			else
				WEB_LoginPage(conn);
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

DWORD WINAPI ConfigWorkerThread(LPVOID lpParam)
{
	while(true)
	{
		// Carrega as configurações
		char bind_ghpserver[255];
		char bind_webserver[255];
		char auth_user[255];
		char auth_password[255];
		char game_serial[255];
		char lang[255];
		char status_notify[4096];
		char acc_ban_url[4096];

		GetPrivateProfileStringA("BIND", "ghpserver", "55507", bind_ghpserver, 255, ".\\GHPServer.ini");
		GetPrivateProfileStringA("BIND", "webserver", "55508", bind_webserver, 255, ".\\GHPServer.ini");
		max_connections = GetPrivateProfileIntA("BIND", "max_connections", 1000, ".\\GHPServer.ini");

		GetPrivateProfileStringA("AUTH", "user", "admin", auth_user, 255, ".\\GHPServer.ini");
		GetPrivateProfileStringA("AUTH", "password", "1234", auth_password, 255, ".\\GHPServer.ini");

		GetPrivateProfileStringA("CFG", "lang", "pt", lang, 255, ".\\GHPServer.ini");
		
		if (std::string("pt") == lang)
		{
			LOGIN_BUTTON_LNG = "Entrar";
			PLAYERS_ON_LNG = "Jogadores Online";
			ALL_PLAYERS_LNG = "Todos jogadores";
			OPTIONS_LNG = "Configurações";
			REMAINING_DAYS_LNG = "dia(s) restantes";

			NO_LONGER_ONLINE_LNG = "O jogador selecionado não está mais online no servidor!";
			BLCK_BY_ADMIN_REASON_LNG = "Bloqueado pelo administrador";
			BLCK_HACK_USE_LNG = "Uso de hack";

			USER_BAN_ERROR_LNG = "O jogador não está online dentro do jogo.";
			USER_BAN_OK_LNG = "A conta foi banida com sucesso!";
			HWID_BAN_ERROR_LNG = "Não foi possível bloquear um ou mais HWID(s), tente novamente.";
			HWID_BAN_OK_LNG = "Todos os HWID's foram banidos com sucesso!";
			IP_BAN_OK_LNG = "O IP do jogador foi banido com sucesso!";

			UNEXPECTED_ERROR_LNG = "Erro inesperado, tente novamente.";
			DISABLED_FUNCTION_LNG = "Essa função está desativada no momento.";
			PLAYER_DISCONNECTED_LNG = "O jogador foi desconectado com sucesso!";
			SCREENSHOT_REQUEST_OK_LNG = "A screenshot foi requisitada com sucesso!";
			SCREENSHOT_REQUEST_ERROR_LNG = "Aguarde 15 segundos antes de requisitar outra SS!";

			PLAYER_INFO_LNG = "Informações";
			PLAYER_OP_LNG = "Opções";
			PLAYER_LOGS_LNG = "Log";
			PLAYER_SS_LNG = "Capturas de tela";

			DELETE_LOGS_LNG = "Apagar logs";
			DELETE_SS_LNG = "Apagar capturas de telas";

			HWID_BAN_HINT_LNG = "Essa operação é usada para banir todos os computadores do jogador.";
			USER_BAN_HINT_LNG = "Essa operação é usada para banir a conta do jogador.";
			IP_BAN_HINT_LNG = "Essa operação é usada para banir o ip do jogador.";
			SS_HINT_LNG = "Essa operação é usada para tirar uma foto da tela do jogador.";
			DC_HINT_LNG = "Essa operação é usada para desconectar o jogador.";
			REFRESH_HINT_LNG = "Essa operação é usada para atualizar a página atual";

			HWID_BAN_BT_LNG = "BANIR HWID's";
			USER_BAN_BT_LNG = "BANIR CONTA";
			IP_BAN_BT_LNG = "BANIR IP";
			SS_BT_LNG = "SCREENSHOT";
			DC_BT_LNG = "DESCONECTAR";
			REFRESH_BT_LNG = "ATUALIZAR";

			NEVER_PLAYED_LNG = "Nunca jogou no servidor";
			MINUTE_LNG = "minuto";
			HOUR_LNG = "hora";
			DAY_LNG = "dia";
			BEFORE_LNG = "atrás";
		}
		else
		{
			LOGIN_BUTTON_LNG = "Login";
			PLAYERS_ON_LNG = "Players Online";
			ALL_PLAYERS_LNG = "All players";
			OPTIONS_LNG = "Options";
			REMAINING_DAYS_LNG = "day(s) remaining";

			NO_LONGER_ONLINE_LNG = "The selected player is no longer online on the server!";
			BLCK_BY_ADMIN_REASON_LNG = "Blocked by the administrator";
			BLCK_HACK_USE_LNG = "Using cheat";

			USER_BAN_ERROR_LNG = "The player is not connected into the game.";
			USER_BAN_OK_LNG = "The ACCOUNT was banned successfully!";
			HWID_BAN_ERROR_LNG = "An unexpected error has occurred, try again.";
			HWID_BAN_OK_LNG = "All HWID's were banned successfully!";
			IP_BAN_OK_LNG = "The player IP was banned successfully!";

			UNEXPECTED_ERROR_LNG = "An unexpected error has occurred, try again!";
			DISABLED_FUNCTION_LNG = "This feature is currently disabled!";
			PLAYER_DISCONNECTED_LNG = "The player has been disconnected successfully!";
			SCREENSHOT_REQUEST_OK_LNG = "The screenshot was successfully requested!";
			SCREENSHOT_REQUEST_ERROR_LNG = "Wait 15 seconds before requesting another SS!";

			PLAYER_INFO_LNG = "Information";
			PLAYER_OP_LNG = "Management";
			PLAYER_LOGS_LNG = "Logs";
			PLAYER_SS_LNG = "Screenshots";

			DELETE_LOGS_LNG = "Delete logs";
			DELETE_SS_LNG = "Delete screenshots";

			HWID_BAN_HINT_LNG = "This operation is used to ban the player's computer.";
			USER_BAN_HINT_LNG = "This operation is used to ban the player's account.";
			IP_BAN_HINT_LNG = "This operation is used to ban the player's ip.";
			SS_HINT_LNG = "This operation take a screenshot of the player's computer.";
			DC_HINT_LNG = "This operation is used to disconnect the player.";
			REFRESH_HINT_LNG = "This operation refresh the current page.";

			HWID_BAN_BT_LNG = "BAN HWID's";
			USER_BAN_BT_LNG = "BAN ACCOUNT";
			IP_BAN_BT_LNG = "BAN IP";
			SS_BT_LNG = "SCREENSHOT";
			DC_BT_LNG = "DISCONNECT";
			REFRESH_BT_LNG = "REFRESH";

			NEVER_PLAYED_LNG = "Never played on the server";
			MINUTE_LNG = "minute";
			HOUR_LNG = "hour";
			DAY_LNG = "day";
			BEFORE_LNG = "ago";
		}

		if (GetPrivateProfileIntA("CFG", "acc_multisession", 0, ".\\GHPServer.ini") == 1)
			acc_multisession = true;
		else
			acc_multisession = false;

		if (GetPrivateProfileIntA("CFG", "hwid_multisession", 0, ".\\GHPServer.ini") == 1)
			hwid_multisession = true;
		else
			hwid_multisession = false;

		if (GetPrivateProfileIntA("AUTOBAN", "acc", 0, ".\\GHPServer.ini") == 1)
			acc_autoban = true;
		else
			acc_autoban = false;

		if (GetPrivateProfileIntA("AUTOBAN", "hwid", 0, ".\\GHPServer.ini") == 1)
			hwid_autoban = true;
		else
			hwid_autoban = false;

		if (GetPrivateProfileIntA("AUTOBAN", "ip", 0, ".\\GHPServer.ini") == 1)
			ip_autoban = true;
		else
			ip_autoban = false;

		ss_quality = GetPrivateProfileIntA("SS", "quality", 25, ".\\GHPServer.ini");
		ss_grayscale = GetPrivateProfileIntA("SS", "grayscale", 0, ".\\GHPServer.ini");
		auto_ss_interval = GetPrivateProfileIntA("SS", "auto_ss_interval", 0, ".\\GHPServer.ini");

		GetPrivateProfileStringA("MU", "serial", "0000000000000000", game_serial, 255, ".\\GHPServer.ini");

		if (GetPrivateProfileIntA("API", "force_api", 0, ".\\GHPServer.ini") == 1)
			force_api = true;
		else
			force_api = false;

		if (GetPrivateProfileIntA("API", "call_api", 0, ".\\GHPServer.ini") == 1)
			call_api = true;
		else
			call_api = false;

		GetPrivateProfileStringA("API", "status_notify", "", status_notify, 4096, ".\\GHPServer.ini");
		GetPrivateProfileStringA("API", "acc_ban_url", "", acc_ban_url, 4096, ".\\GHPServer.ini");

		BIND_GHPSERVER = atoi(bind_ghpserver);;
		BIND_WEBSERVER = bind_webserver;
		AUTH_USER = auth_user;
		AUTH_PASSWORD = auth_password;
		GAME_SERIAL = game_serial;
		STATUS_NOTIFY = status_notify;
		ACC_BAN_URL = acc_ban_url;

		// Corrige a porta
		if (BIND_GHPSERVER <= 0 || BIND_GHPSERVER >= 65000)
			BIND_GHPSERVER = 55507;

		// Aguarda para abrir dados novamente
		Sleep(10000);
	}

	return 0;
}

DWORD WINAPI WebServerWorkerThread(LPVOID lpParam)
{
	struct mg_server * server;
	
	server = mg_create_server(NULL, server_request);
	mg_set_option(server, "listening_port", BIND_WEBSERVER.c_str());
		
	while (true)
	{
		mg_poll_server(server, 1000);
	}

	mg_destroy_server(&server);

	return 0;
}

std::vector<std::string> split(std::string s, char delim)
{
	std::vector<std::string> elems;

    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
	{
        elems.push_back(item);
    }
    return elems;
}

DWORD WINAPI MasterServerWorkerThread(LPVOID lpParam)
{
	// Conecta no masterserver
	std::string ret = "";
	ghp_NET::HTTPClient http;
	bool error = true, logged = false;
	
	while(true)
	{
		// Converte o número de players para texto
		char second_part[255];
		sprintf_s(second_part, "&p=%d", players.size());

		for (int i = 0; i < 3; i++)
		{
			// Se conectar, sai do loop
			if (http.GetURLToString(VMProtectDecryptStringA(MASTERSERVER_LINK) + C_REF + second_part, &ret))
			{
				error = false;
				break;
			}

			if (i == 2)
			{
				error = true;
				srvLog.LineOut(true, "An error has occurred while trying to connect to MasterServer (check for updates)");
			}
		}

		if (!error)
		{
			// Abre o XML
			tinyxml2::XMLDocument doc;

			if (doc.Parse(ret.c_str(), ret.size()) != tinyxml2::XML_SUCCESS)
			{
				error = true;
				srvLog.LineOut(true, "MasterServer data corrupted (check for updates)");
			}
			else
			{
				// Procura o elemento inicial
				tinyxml2::XMLElement * element = doc.FirstChild()->ToElement();

				if (element == 0)
				{
					error = true;
					srvLog.LineOut(true, "MasterServer data corrupted (check for updates)");
				}
				else
				{
					// Adiciona os dados no vetor
					C_DATA.clear();
					C_DATA.push_back(element->Attribute("srvname"));
					C_DATA.push_back(element->Attribute("srvip"));
					C_DATA.push_back(element->Attribute("name"));
					C_DATA.push_back(element->Attribute("email"));
					C_DATA.push_back(element->Attribute("update"));
					doc.Clear();

					// Verifica se recebeu os dados corretamente
					if (C_DATA.size() != 5)
					{
						error = true;
						srvLog.LineOut(true, "MasterServer data corrupted (check for updates)");
					}
					else
					{
						if (!logged)
						{
							logged = true;
							srvLog.LineOut(true, "MasterServer data retrieved: %s at %s owned by %s (%s)", C_DATA[0].c_str(), C_DATA[1].c_str(), C_DATA[2].c_str(), C_DATA[3].c_str());
						}
					}
				}
			}
		}

		Sleep(600000); // 10 minutos
	}
}

void LoadCRC()
{
	std::vector<ghp_UTILS::CRC> tempcrc;
	std::vector<std::wstring> files;

	files.clear();

	ghp_UTILS::GetAllFiles(files, L".\\data\\crc\\");

	UCHAR sha1[20];

	for (UINT i = 0; i < files.size(); i++)
	{
		if (!ghp_CRYPTO::CalculeFileSHA1(files[i], &sha1[0]))
		{
			std::string sname = std::string(files[i].begin(), files[i].end());
			srvLog.LineOut(true, "[CRC] An error has occurred while hashing: %s", sname.c_str());
		}
		else
		{
			// remove a parte inicial da string
			files[i].erase(files[i].begin() + 2, files[i].begin() + 11);

			ghp_UTILS::CRC temp;
			temp.fileName = files[i];
			memcpy_s(temp.crc, 20, sha1, 20);
			
			tempcrc.push_back(temp);
		}
	}

	crc = tempcrc;
	srvLog.LineOut(true, "[CRC] %d file(s) loaded!", crc.size());
}

bool IsEXE(std::wstring fileName)
{
	int len = fileName.length();

	if ((fileName[len-3] == 'E' || fileName[len-3] == 'e') &&
		(fileName[len-2] == 'X' || fileName[len-2] == 'x') &&
		(fileName[len-1] == 'E' || fileName[len-1] == 'e'))
		return true;
	else
		return false;
}

bool IsDLL(std::wstring fileName)
{
	int len = fileName.length();

	if ((fileName[len-3] == 'D' || fileName[len-3] == 'd') &&
		(fileName[len-2] == 'L' || fileName[len-2] == 'l') &&
		(fileName[len-1] == 'L' || fileName[len-1] == 'l'))
		return true;
	else
		return false;
}

void LoadHacks()
{
	std::vector<ghp_UTILS::Database> temphacks;
	std::vector<std::wstring> files;

	files.clear();
	
	ghp_UTILS::GetAllFiles(files, L".\\data\\hacks\\");

	UCHAR hash[20];

	for (UINT i = 0; i < files.size(); i++)
	{
		if (!ghp_CRYPTO::CalculeFileSHA1(files[i], &hash[0]))
		{
			std::string sname = std::string(files[i].begin(), files[i].end());
			srvLog.LineOut(true, "[CHEATS] An error has occurred while blocking: %s", sname.c_str());
		}
		else
		{
			if (IsEXE(files[i]))
			{
				ghp_UTILS::Database db;
				db.type = DB_SHA1_HACK;
				memcpy_s(db.data, 20, hash, 20);
				temphacks.push_back(db);
			}
			else if (IsDLL(files[i]))
			{
				ghp_UTILS::Database db;
				db.type = DB_SHA1_DLL;
				memcpy_s(db.data, 20, hash, 20);
				temphacks.push_back(db);
			}
		}
	}

	hacks = temphacks;
	srvLog.LineOut(true, "[CHEATS] %d cheats(s) added!", hacks.size());
}

void LoadCheatWhitelist()
{
	ghp_UTILS::DataChunk chunk;

	if (!ghp_UTILS::LoadFileData(L"CheatWhitelist.txt", &chunk))
	{
		remove("CheatWhitelist.bin"); // remove o bin
		srvLog.LineOut(true, "[CHEAT_WHITELIST] CheatWhitelist.txt not found!");
		chunk.FreeAll();
		return;
	}

	// Captura todos os ID's
	std::stringstream ss(chunk.toString());
	std::vector<DWORD> items;
	std::string item;

	while(std::getline(ss, item))
	{
		DWORD id = (DWORD)atoi(item.c_str());

		if (id != 0)
			items.push_back(ghp_CRYPTO::EncryptDWORD(id));
		else
			srvLog.LineOut(true, "[CHEAT_WHITELIST] ID \"%s\" invalid!", item.c_str());
	}

	// Se não tiver nada para desbloquear, simplesmente sai fora
	if (items.size() == 0)
	{
		srvLog.LineOut(true, "[CHEAT_WHITELIST] Nothing to do!", items.size());
		remove("CheatWhitelist.bin"); // remove o bin
		return;
	}

	VMProtectBeginUltra("LoadCheatWhitelist");
	ghp_UTILS::StructuredFile wl;

	// Cria o arquivo estruturado
	if (!wl.Set(items.size(), sizeof(DWORD), items.data()) || !wl.SaveToFile(VMProtectDecryptStringW(L"CheatWhitelist.bin"), PRV_KEY.toString(), (UCHAR*)GHP_AES_KEY))
	{
		remove("CheatWhitelist.bin"); // remove o bin
		srvLog.LineOut(true, "[CHEAT_WHITELIST] Save BIN error!", item.c_str());
		chunk.FreeAll();
		return;
	}

	// Verifica se foi criado com sucesso
	if (!wl.LoadFromFile(VMProtectDecryptStringW(L"CheatWhitelist.bin"), PUB_KEY.toString(), (UCHAR*)GHP_AES_KEY) || wl.GetLength() != items.size())
	{
		remove("CheatWhitelist.bin"); // remove o bin
		srvLog.LineOut(true, "[CHEAT_WHITELIST] BIN invalid!", item.c_str());
		chunk.FreeAll();
		return;
	}
	VMProtectEnd();

	srvLog.LineOut(true, "[CHEAT_WHITELIST] %d cheats(s) added!", items.size());
	chunk.FreeAll();
}

void LoadAPIWhitelist()
{
	ghp_UTILS::DataChunk chunk;

	if (!ghp_UTILS::LoadFileData(L"APIWhitelist.txt", &chunk))
	{
		remove("APIWhitelist.bin"); // remove o bin
		srvLog.LineOut(true, "[API_WHITELIST] APIWhitelist.txt not found!");
		chunk.FreeAll();
		return;
	}

	// Captura todos os ID's
	std::stringstream ss(chunk.toString());
	std::vector<DWORD> items;
	std::string item;

	while(std::getline(ss, item))
	{
		DWORD id = (DWORD)atoi(item.c_str());

		if (id != 0)
			items.push_back(ghp_CRYPTO::EncryptDWORD(id));
		else
			srvLog.LineOut(true, "[API_WHITELIST] ID \"%s\" invalid!", item.c_str());
	}

	// Se não tiver nada para desbloquear, simplesmente sai fora
	if (items.size() == 0)
	{
		srvLog.LineOut(true, "[API_WHITELIST] Nothing to do!", items.size());
		remove("APIWhitelist.bin"); // remove o bin
		return;
	}

	VMProtectBeginUltra("APIWhitelist");
	ghp_UTILS::StructuredFile wl;

	// Cria o arquivo estruturado
	if (!wl.Set(items.size(), sizeof(DWORD), items.data()) || !wl.SaveToFile(VMProtectDecryptStringW(L"APIWhitelist.bin"), PRV_KEY.toString(), (UCHAR*)GHP_AES_KEY))
	{
		remove("APIWhitelist.bin"); // remove o bin
		srvLog.LineOut(true, "[API_WHITELIST] Save BIN error!", item.c_str());
		chunk.FreeAll();
		return;
	}

	// Verifica se foi criado com sucesso
	if (!wl.LoadFromFile(VMProtectDecryptStringW(L"APIWhitelist.bin"), PUB_KEY.toString(), (UCHAR*)GHP_AES_KEY) || wl.GetLength() != items.size())
	{
		remove("APIWhitelist.bin"); // remove o bin
		srvLog.LineOut(true, "[API_WHITELIST] BIN invalid!", item.c_str());
		chunk.FreeAll();
		return;
	}
	VMProtectEnd();

	srvLog.LineOut(true, "[API_WHITELIST] %d api(s) added!", items.size());
	chunk.FreeAll();
}

DWORD WINAPI CRCandHACKSWorkerThread(LPVOID lpParam)
{
	while(true)
	{
		LoadCRC();
		LoadHacks();
		Sleep(600000); // 10 minutos
	}
}

DWORD WINAPI CheatWhitelistWorkerThread(LPVOID lpParam)
{
	while(true)
	{
		LoadCheatWhitelist();
		Sleep(600000); // 10 minutos
	}
}

DWORD WINAPI APIWhitelistWorkerThread(LPVOID lpParam)
{
	while(true)
	{
		LoadAPIWhitelist();
		Sleep(600000); // 10 minutos
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
    {
		case CALLBACK_MESSAGE:
		{
			if (lParam == WM_RBUTTONDOWN)
			{
				HMENU popMenu = CreatePopupMenu();
				AppendMenu(popMenu, MF_STRING, ID_MENU_EXIT, L"Stop GHPServer");
				
				POINT pCursor;
				GetCursorPos(&pCursor);
				
				TrackPopupMenu(popMenu, TPM_LEFTBUTTON | TPM_RIGHTALIGN, pCursor.x, pCursor.y, 0, hWnd, NULL);
				DestroyMenu(popMenu);
			}
		}
		break;
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == ID_MENU_EXIT)
			{
				Shell_NotifyIcon(NIM_DELETE, &NID);
				ExitProcess(0);
			}
		}
		break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
    }
	
	return DefWindowProc (hWnd, message, wParam, lParam);
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	VMProtectBeginUltra("wWinMain");
	// Inicia o C_DATA default
	while(C_DATA.size() < 5)
	{
		C_DATA.push_back("-");
	}

	// Carrega as chaves públicas e privadas
	if (!ghp_UTILS::LoadFileData(L"PUB.KEY", &PUB_KEY) || !ghp_UTILS::LoadFileData(L"PRV.KEY", &PRV_KEY))
	{
		srvLog.LineOut(true, "Invalid Keys!");
		return 0;
	}

	// Inicia o CPU %
	PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

	// Carrega os arquivos do update
	ghp_UTILS::LoadFileData(L".\\data\\update\\DLL", &GHP_DLL);
	ghp_UTILS::LoadFileData(L".\\data\\update\\DB", &GHP_DB);
	ghp_UTILS::LoadFileData(L".\\data\\update\\LIC", &GHP_LIC);
	DLL_HASH = ghp_CRYPTO::GetFormatedSHA1(&GHP_DLL);
	DB_HASH = ghp_CRYPTO::GetFormatedSHA1(&GHP_DB);
	LIC_HASH = ghp_CRYPTO::GetFormatedSHA1(&GHP_LIC);

	// Cria a referência do cliente (para comunicação com o MasterServer e criação dos pagamentos)
	C_REF = ghp_CRYPTO::GetFormatedSHA1(&PRV_KEY);
	srvLog.LineOut(true, "C_REF initialized: %s", C_REF.c_str());

	// Inicia a thread das configurações
	CreateThread(NULL, 0, ConfigWorkerThread, NULL, 0, NULL); Sleep(200);
	// Inicia a thread de conexões
	CreateThread(NULL, 0, ServerWorkerThread, NULL, 0, NULL); Sleep(200);
	// Inicia a thread dos players
	CreateThread(NULL, 0, PlayerCheckWorkerThread, NULL, 0, NULL); Sleep(200);
	// Inicia a thread do WebServer
	CreateThread(NULL, 0, WebServerWorkerThread, NULL, 0, NULL); Sleep(200);
	// Inicia a thread de conexão com o MasterServer
	CreateThread(NULL, 0, MasterServerWorkerThread, NULL, 0, NULL); Sleep(200);
	// Inicia a thread que lê os arquivos do CRC e dos hacks
	CreateThread(NULL, 0, CRCandHACKSWorkerThread, NULL, 0, NULL); Sleep(200);
	// Inicia a thread que desbloqueia os bangs
	CreateThread(NULL, 0, CheatWhitelistWorkerThread, NULL, 0, NULL);  Sleep(200);
	// Inicia a thread que desbloqueia a memoria
	CreateThread(NULL, 0, APIWhitelistWorkerThread, NULL, 0, NULL); Sleep(200);

	// Inicia o Window
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"ghp_server";
	wc.hIcon = (HICON)LoadImage(hInstance, L"MAINICON", IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL, L"ghp_server", L"ghp_server", WS_OVERLAPPEDWINDOW, 0, 0, 2, 2, NULL, NULL, hInstance, NULL);

	// Inicia o ícone na área de notificação
	NID.cbSize = sizeof(NID);
	NID.hIcon = wc.hIcon;
	NID.uCallbackMessage = CALLBACK_MESSAGE;
	NID.hWnd = hWnd;
	NID.uID = WM_USER + 2;
	memcpy_s(NID.szTip, 128, VMProtectDecryptStringW(GHP_SERVER_TITLE), std::wstring(VMProtectDecryptStringW(GHP_SERVER_TITLE)).length() * sizeof(wchar_t));
	NID.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE;
	Shell_NotifyIcon(NIM_ADD, &NID);
	VMProtectEnd();
	
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
    {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
    }
	
	return 0;
}