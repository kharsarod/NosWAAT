#include "stdafx.h"
#include "TGroup.h"

//Typedefinitionen
TGroup Group;
//------------------------------------------------------------

TGroup::TGroup(void)
{
}

//------------------------------------------------------------

TGroup::~TGroup(void)
{
}
//------------------------------------------------------------
void TGroup::pjoin_handle(TSocket *Client, int action, int partnerid)
{
	switch(action)
	{
	case ACTION_GP_INVITE:
		this->Send_Invite(Client, partnerid);
		break;
	case ACTION_GP_INVITE_EX:
		this->Send_Invite(Client, partnerid);
		break;
	case ACTION_GP_ACCEPT:
		this->Invite_Accept(Client, partnerid);
		break;
	case ACTION_GP_DECLINE:
		this->Invite_Decline(Client, partnerid);
		break;
	}
}

//------------------------------------------------------------

void TGroup::Player_SendStats(int mapindex, int playerindex)
{
	if(Map.MapData[mapindex].Players[playerindex]->Character->Group.ingroup)
	{
		int time = GetTickCount() / 1000;
		if(time >= Map.MapData[mapindex].Players[playerindex]->Character->Group.next_stat_send)
		{
			int groupid = Map.MapData[mapindex].Players[playerindex]->Character->Group.groupid;
			int allocated_group = this->Group_GetByID(groupid);
			int groupsize = this->groups[allocated_group].members.size();
			int memberindex = -1;

			std::stringstream message;
			//pst 1 558097 2 100 100 221 60 0 1 0
			int charid = Map.MapData[mapindex].Players[playerindex]->Character->CharacterInfo.ID;
			int health = (Map.MapData[mapindex].Players[playerindex]->Character->Status.currenthp * 100) / Map.MapData[mapindex].Players[playerindex]->Character->Status.maxhp;
			int maxhp = Map.MapData[mapindex].Players[playerindex]->Character->Status.maxhp;
			int mana = (Map.MapData[mapindex].Players[playerindex]->Character->Status.currentmp * 100) / Map.MapData[mapindex].Players[playerindex]->Character->Status.maxmp;
			int maxmp = Map.MapData[mapindex].Players[playerindex]->Character->Status.maxmp;

			for(int i = 0; i < groupsize; i++)
			{
				if(this->groups[allocated_group].members[i].client->Character->CharacterInfo.ID == charid)
				{
					memberindex = i + 1;
					break;
				}
			}

			message << "pst 1 " << charid << " " << memberindex << " " << health << " " << mana << " " << maxhp << " " << maxmp << " 0 1 0";

			for(int i = 0; i < groupsize; i++)
			{
				if(this->groups[allocated_group].members[i].client->Character->CharacterInfo.ID != charid)
				{
					this->groups[allocated_group].members[i].client->Send(message.str());
				}
			}

			Map.MapData[mapindex].Players[playerindex]->Character->Group.next_stat_send = time + 2;
		}
	}
}

//------------------------------------------------------------

int TGroup::Group_Create(int leaderid)
{
	int allocated_id = -1;

	int groupid = leaderid + General.CreateRandomNumber(1, 100000);
	structGroups gruppe;
	gruppe.groupid = groupid;
	gruppe.leadercharid = leaderid;

	allocated_id = this->groups.size();
	this->groups.push_back(gruppe);

	return allocated_id;
}
//------------------------------------------------------------
int TGroup::Group_GetByID(int groupid)
{
	int groupsize = this->groups.size();

	for(int i = 0; i < groupsize; i++)
	{
		if(this->groups[i].groupid == groupid)
		{
			return i;
		}
	}

	return -1;
}
//------------------------------------------------------------
bool TGroup::Leader_Check(TSocket *Client)
{
	if(Client->Character->Group.ingroup)
	{
		int groupsize = this->groups.size();
		for(int i = 0; i < groupsize; i++)
		{
			if(this->groups[i].groupid == Client->Character->Group.groupid)
			{
				if(this->groups[i].leadercharid == Client->Character->CharacterInfo.ID)
				{
					return true;
				}
			}
		}
	}

	return false;
}
//------------------------------------------------------------
bool TGroup::Request_Check(TSocket *Client, int partnerid)
{
	int size = Client->Character->Group.requests.size();

	for(int i = 0; i < size; i++)
	{
		if(Client->Character->Group.requests[i] == partnerid)
		{
			return true;
		}
	}

	return false;
}
//------------------------------------------------------------
int TGroup::Get_Groupsize(int groupid)
{
	int groupsize = this->groups.size();

	for(int i = 0; i < groupsize; i++)
	{
		if(this->groups[i].groupid == groupid)
		{
			return this->groups[i].members.size();
		}
	}
}
//------------------------------------------------------------
void TGroup::Send_Invite(TSocket *Client, int partnerid)
{
	std::stringstream packet;
	int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
	int MapPlayersSize = Map.MapData[MapIndex].Players.size();

	//Schauen ob er sich nicht selber schickt
	if(Client->Character->CharacterInfo.ID != partnerid)
	{
		bool ingroupnleader = (Client->Character->Group.ingroup && this->Leader_Check(Client));

		//Falls nicht in Gruppe ODER in Gruppe & Leader
		if(Client->Character->Group.ingroup == false || ingroupnleader)
		{

			//Alle Maps absuchen nach dem Spieler
			for(auto i = 0; i < MapPlayersSize; i++)
			{
				//Wenn die ID des Spielers übereinstimmt, einladen
				if(Map.MapData[MapIndex].Players[i]->Character->CharacterInfo.ID == partnerid)
				{
					//Request besteht bereits?
					if(!this->Request_Check(Client, partnerid))
					{
						bool blocked = false; //TODO: Blacklist check
						if(!blocked)
						{
							packet << "dlg #pjoin^" << ACTION_GP_ACCEPT << "^" << Client->Character->CharacterInfo.ID << " #pjoin^"
								<< ACTION_GP_DECLINE << "^" << Client->Character->CharacterInfo.ID << " [" << Client->Character->CharacterInfo.Name
								<< "] has invited you into a group";
							Map.MapData[MapIndex].Players[i]->Send(packet.str());
							packet.str("");
							packet << Map.MapData[MapIndex].Players[i]->Character->CharacterInfo.Name << " has been invited";
							Message.Message_Info(Client, packet.str());

							//Add Requests
							Client->Character->Group.requests.push_back(partnerid);
							Map.MapData[MapIndex].Players[i]->Character->Group.requests.push_back(Client->Character->CharacterInfo.ID);
							return;
						}
					}else
					{
						Message.Message_Info(Client, "You've allready invited this player.");
					}

					break;
				}
			}
		} 
	}

}
//------------------------------------------------------------
void TGroup::Send_Group(TSocket *Client)
{
	std::stringstream message;
	//pinit 2 1|825728|2|1|,knox|2|1|0|0 1|825479|3|1|KnoxLiebtsAnal|2|1|0|0
	//		  <Type><ID><Slot><Level><Name><?><VNUM><?><?>
	int groupsize = this->Get_Groupsize(Client->Character->Group.groupid);
	int allocated_group = this->Group_GetByID(Client->Character->Group.groupid);

	message << "pinit " << groupsize;

	for(int i = 0; i < groupsize; i++)
	{
		int memberID = this->groups[allocated_group].members[i].client->Character->CharacterInfo.ID;
		std::string memberName = this->groups[allocated_group].members[i].client->Character->CharacterInfo.Name;

		message << " 1|" << memberID << "|" << i + 1 << "|1|" << memberName << "|2|1|0|0";
	}

	Client->Send(message.str());
}
//------------------------------------------------------------
void TGroup::Invite_Accept(TSocket *Client, int partnerid)
{
	std::stringstream packet;
	int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
	int MapPlayersSize = Map.MapData[MapIndex].Players.size();

	//Schauen ob er sich nicht selber schickt
	if(Client->Character->CharacterInfo.ID != partnerid && !Client->Character->Group.ingroup)
	{
		//Request besteht?
		if(this->Request_Check(Client, partnerid))
		{
			//Alle Maps absuchen nach dem Spieler
			for(auto i = 0; i < MapPlayersSize; i++)
			{
				//Wenn die des Spielers übereinstimmt, akzeptieren
				if(Map.MapData[MapIndex].Players[i]->Character->CharacterInfo.ID == partnerid)
				{
					structGroupMembers member;

					//Es besteht noch keine Gruppe
					if(!Map.MapData[MapIndex].Players[i]->Character->Group.ingroup)
					{

						int GroupID = this->Group_Create(Map.MapData[MapIndex].Players[i]->Character->CharacterInfo.ID);

						//Der Inviter
						Map.MapData[MapIndex].Players[i]->Character->Group.groupid = this->groups[GroupID].groupid;
						Map.MapData[MapIndex].Players[i]->Character->Group.ingroup = true;
						Map.MapData[MapIndex].Players[i]->Send("info You're the leader");

						//Eingeladene
						Client->Character->Group.groupid = this->groups[GroupID].groupid;
						Client->Character->Group.ingroup = true;

						member.client = Client;
						Client->Character->Group.next_stat_send = 0;
						this->groups[GroupID].members.push_back(member);

						member.client = Map.MapData[MapIndex].Players[i];
						Map.MapData[MapIndex].Players[i]->Character->Group.next_stat_send = 0;
						this->groups[GroupID].members.push_back(member);

						//Send Grouplist
						this->Send_Group(Client);
						this->Send_Group(Map.MapData[MapIndex].Players[i]);

						//this->Requests_Delete(Client, partnerid);

						this->Requests_Delete(Client, Map.MapData[MapIndex].Players[i]);

					}else //gruppe besteht schon
					{


						Client->Character->Group.ingroup = true;
						Client->Character->Group.groupid = Map.MapData[MapIndex].Players[i]->Character->Group.groupid;

						int groupsize = this->Get_Groupsize(Map.MapData[MapIndex].Players[i]->Character->Group.groupid);
						int allocated_group = this->Group_GetByID(Map.MapData[MapIndex].Players[i]->Character->Group.groupid);

						member.client = Client;
						Client->Character->Group.next_stat_send = 0;
						this->groups[allocated_group].members.push_back(member);

						groupsize = this->Get_Groupsize(Map.MapData[MapIndex].Players[i]->Character->Group.groupid);

						for(int t = 0; t < groupsize; t++)
						{
							//Send Group list
							this->Send_Group(this->groups[allocated_group].members[t].client);
						}

						this->Requests_Delete(Client, Map.MapData[MapIndex].Players[i]);

					}

					break;
				}
			}
		}
	}
}
//------------------------------------------------------------
void TGroup::Invite_Decline(TSocket *Client, int partnerid)
{
	std::stringstream message;
	structMapData currentMap;
	int mapsize = Map.MapData.size();

	for(auto i = 0; i < mapsize; i++)
	{
		//Wenn Map gefunden zuweisen
		if(Client->Character->Map.id == Map.MapData[i].id)
		{
			currentMap = Map.MapData[i];
			break;
		}
	}

	int size;

	//Check ob er sich selber ablehnt
	if(partnerid != Client->Character->CharacterInfo.ID)
	{
		//Alle Spieler auf der Map ablaufen
		size = currentMap.Players.size();

		for(auto e = 0; e < size; e++)
		{
			//Wenn Spieler gefunden
			if(currentMap.Players[e]->Character->CharacterInfo.ID == partnerid)
			{
				//Schauen ob Request vorhanden
				bool request_done = this->Requests_Delete(Client, currentMap.Players[e]);

				if(request_done)
				{
					//Trade Ablehnung schicken
					message << "say 1 " << currentMap.Players[e]->Character->CharacterInfo.ID << " 10 " << Client->Character->CharacterInfo.Name << " declined the grouprequest";
					currentMap.Players[e]->Send(message.str());
				}
				break;
			}
		}
	}
}
//------------------------------------------------------------
bool TGroup::Requests_Delete(TSocket *Client, TSocket *Partner)
{
	int ClientID = Client->Character->CharacterInfo.ID;
	int PartnerID = Partner->Character->CharacterInfo.ID;

	int size = Client->Character->Group.requests.size();

	bool ClientDel = false;

	for(int i = 0; i < size; i++)
	{
		if(Client->Character->Group.requests[i] == PartnerID)
		{
			//Requests löschen
			Client->Character->Group.requests.erase(Client->Character->Group.requests.begin() + i);
			ClientDel = true;
		}
	}

	bool PartnerDel = false;
	size = Partner->Character->Group.requests.size();

	for(int i = 0; i < size; i++)
	{
		if(Partner->Character->Group.requests[i] == ClientID)
		{
			//Requests löschen
			Partner->Character->Group.requests.erase(Partner->Character->Group.requests.begin() + i);
			PartnerDel = true;
		}
	}

	if(ClientDel && PartnerDel)
	{
		return true;
	}else
	{
		return false;
	}
}
//------------------------------------------------------------
void TGroup::Leave_Group(TSocket *Client)
{
	if(Client->Character->Group.ingroup)
	{
		int groupsize = this->Get_Groupsize(Client->Character->Group.groupid);
		int allocated_group = this->Group_GetByID(Client->Character->Group.groupid);
		bool isleader = this->Leader_Check(Client);

		bool newleaderset = false;

		for(int i = 0; i < groupsize; i++)
		{
			if(this->groups[allocated_group].members[i].client == Client)
			{
				this->groups[allocated_group].members.erase(this->groups[allocated_group].members.begin() + i);
				Client->Character->Group.ingroup = false;
				Client->Send("pinit 0");
				break;
			}
		}

		groupsize = this->Get_Groupsize(Client->Character->Group.groupid);

		if(groupsize == 1) //Gruppe leer
		{
			this->Leave_Group(this->groups[allocated_group].members[0].client);

		}else if(groupsize > 1)
		{
			for(int i = 0; i < groupsize; i++)
			{
				if(isleader && !newleaderset)
				{
					this->groups[allocated_group].leadercharid = this->groups[allocated_group].members[i].client->Character->CharacterInfo.ID;
					newleaderset = true;
					this->groups[allocated_group].members[i].client->Send("info You're the new leader");
				}

				this->Send_Group(this->groups[allocated_group].members[i].client);
			}
		}else if(groupsize == 0)
		{
			//Gruppe löschen
			this->groups.erase(this->groups.begin() + allocated_group);
		}
	}

}
//------------------------------------------------------------