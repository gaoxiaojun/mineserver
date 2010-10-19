
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <deque>
#include <SocketHandler.h>
#include <ListenSocket.h>
#include "tri_logger.hpp"
#include "DisplaySocket.h"
#include "StatusHandler.h"

#include "map.h"
#include "user.h"
    

    extern ListenSocket<DisplaySocket> l;
    extern StatusHandler h;

    std::vector<User> Users;


    User::User(SOCKET sock, uint32 EID)
    {
      this->action=0;
      this->waitForData=false;
      this->sock=sock;
      this->UID=EID;
      this->logged=false;
      this->admin=false;
      
      /*
      //Send signal to create an entity
      uint8 entityData[5];
      entityData[0]=0x1e; //Initialize entity;
      putSint32(&entityData[1], 12345);
      h.SendSock(GetSocket(), (uint8 *)&entityData[0], 5);
      
      uint8 entityData2[256];  

      int curpos=0;
      entityData2[curpos]=0x14; //Named Entity Spawn
      curpos++;
      putSint32(&entityData2[curpos], 12345);        
      curpos+=4;
      entityData2[curpos]=0;
      entityData2[curpos+1]=6;//nick.size();
      curpos+=2;
      
      //for(int j=0;j<nick.size();j++)
      //{
      //  entityData2[curpos]=nick[j];
      //  curpos++;
      //}
      
      entityData2[curpos]='P';
      entityData2[curpos+1]='s';
      entityData2[curpos+2]='o';
      entityData2[curpos+3]='d';
      entityData2[curpos+4]='e';
      entityData2[curpos+5]='n';
      curpos+=6;
      
      putSint32(&entityData2[curpos],35);
      curpos+=4;
      putSint32(&entityData2[curpos],65*32);
      curpos+=4;
      putSint32(&entityData2[curpos],0);
      curpos+=4;
      entityData2[curpos]=10; //Rotation
      entityData2[curpos+1]=0; //Pitch
      curpos+=2;
      putSint16(&entityData2[curpos],0); //current item
      curpos+=2;
      h.SendSock(GetSocket(), (uint8 *)&entityData2[0], curpos);
      */
      /*
      for(int i=0;i<Users.size();i++)
      {
        //Initialize entity
        putSint32(&entityData[1], Users[i].UID);
        h.SendSock(sock, &entityData[0],5);

        int curpos=0;
        entityData2[0]=0x14; //Named Entity Spawn
        curpos++;
        putSint32(&entityData2[curpos], Users[i].UID);        
        curpos+=4;
        entityData2[curpos]=0;
        entityData2[curpos+1]=Users[i].nick.size();
        curpos+=2;
        for(int j=0;j<Users[i].nick.size();j++)
        {
          entityData2[curpos]=Users[i].nick[j];
          curpos++;
        }
        putSint32(&entityData2[curpos],(int)Users[i].pos.x);
        curpos+=4;
        putSint32(&entityData2[curpos],(int)Users[i].pos.y);
        curpos+=4;
        putSint32(&entityData2[curpos],(int)Users[i].pos.z);
        curpos+=4;
        entityData2[curpos]=10; //Rotation
        entityData2[curpos+1]=(char)Users[i].pos.yaw;
        curpos+=2;
        putSint16(&entityData2[curpos],1);
        curpos+=2;

        h.SendSock(sock, (uint8 *)&entityData2[0], curpos);
      }
      */
      
      
      
      
      //sendOthers(sock, 
      //Send login to all
      //h.SendAll(std::string((char *)&data[0],8+nick.size()));        
    }

    bool User::changeNick(std::string nick, std::deque<std::string> admins)
    {
      this->nick=nick;
      
      // Check adminstatus
      for(int i = 0; i < admins.size(); i++) {
        if(admins[i] == nick) {
            this->admin=true;
            TRI_LOG_STR(nick + " admin");
        }
      }
      
      return true;
    }

    User::~User()
    {
      //Send signal to everyone that the entity is destroyed
      uint8 entityData[5];
      entityData[0]=0x1d; //Destroy entity;
      putSint32(&entityData[1], this->UID);
      this->sendOthers(&entityData[0],5);
    }

    bool User::updatePos(double x, double y, double z, double stance)
    {
         
        //Do we send relative or absolute move values
        if(0)//abs(x-this->pos.x)<127
          //&& abs(y-this->pos.y)<127
          //&& abs(z-this->pos.z)<127)
        {
          uint8 movedata[8];
          movedata[0]=0x1f; //Relative move
          putUint32(&movedata[1],this->UID);
          movedata[5]=(char)(x-this->pos.x);
          movedata[6]=(char)(y-this->pos.y);
          movedata[7]=(char)(z-this->pos.z);
          this->sendOthers(&movedata[0],8);
        }
        else
        {
          uint8 teleportData[19];
          teleportData[0]=0x22; //Teleport
          putSint32(&teleportData[1],this->UID);
          putSint32(&teleportData[5],(int)this->pos.x*32);
          putSint32(&teleportData[9],(int)this->pos.y*32);
          putSint32(&teleportData[13],(int)this->pos.z*32);
          teleportData[17]=(char)this->pos.yaw;
          teleportData[18]=(char)this->pos.pitch;
          this->sendOthers(&teleportData[0],19);
        }
          
          
        this->pos.x=x;
        this->pos.y=y;
        this->pos.z=z;
        this->pos.stance=stance;
        return true;
    }

    bool User::updateLook(float yaw, float pitch)
    {
      
        uint8 lookdata[7];
        lookdata[0]=0x20;
        putUint32(&lookdata[1],this->UID);
        lookdata[5]=(char)(yaw);
        lookdata[6]=(char)(pitch);          
        this->sendOthers(&lookdata[0],7);
          
        this->pos.yaw=yaw;
        this->pos.pitch=pitch;
        return true;

    }

    bool User::sendOthers(uint8* data,uint32 len)
    {
      unsigned int i;
      for(i=0;i<(int)Users.size();i++)
      {
        if(Users[i].sock!=this->sock)
        {
          h.SendSock(Users[i].sock, data,len);
        }
      }
      return true;
    }

    bool User::sendAll(uint8* data,uint32 len)
    {
      unsigned int i;
      for(i=0;i<(int)Users.size();i++)
      {
        if(Users[i].sock)
        {
          h.SendSock(Users[i].sock, data,len);
        }
      }
      return true;
    }


    bool User::teleport(double x, double y, double z)
    {      
      uint8 teleportdata[42]={0};
      int curpos=0;
      teleportdata[curpos]=0x0d;
      curpos++;
      putDouble(&teleportdata[curpos],x); //X
      curpos+=8;
      putDouble(&teleportdata[curpos],y);  //Y
      curpos+=8;
      putDouble(&teleportdata[curpos],z); //Z
      curpos+=8;
      putDouble(&teleportdata[curpos], 0.0); //Stance
      curpos+=8;
      putFloat(&teleportdata[curpos], 0.0);
      curpos+=4;
      putFloat(&teleportdata[curpos], 0.0);
      curpos+=4;
      teleportdata[curpos] = 0; //On Ground
      h.SendSock(this->sock, (char *)&teleportdata[0], 42);

      return true;
    }

    bool User::spawnUser(int x, int y, int z)
    {
       uint8 entityData2[256];
      int curpos=0;
      entityData2[curpos]=0x14; //Named Entity Spawn
      curpos++;
      putSint32(&entityData2[curpos], this->UID);        
      curpos+=4;
      entityData2[curpos]=0;
      entityData2[curpos+1]=this->nick.size();
      curpos+=2;
      
      for(int j=0;j<this->nick.size();j++)
      {
        entityData2[curpos]=this->nick[j];
        curpos++;
      }
      
      putSint32(&entityData2[curpos],x);
      curpos+=4;
      putSint32(&entityData2[curpos],y);
      curpos+=4;
      putSint32(&entityData2[curpos],z);
      curpos+=4;
      entityData2[curpos]=0; //Rotation
      entityData2[curpos+1]=0; //Pitch
      curpos+=2;
      putSint16(&entityData2[curpos],0); //current item
      curpos+=2;
      this->sendOthers((uint8 *)&entityData2[0], curpos);

    }

    bool User::spawnOthers()
    {

      for(int i=0;i<Users.size(); i++)
      {
        if(Users[i].UID!=this->UID && Users[i].nick != this->nick)
        {
          uint8 entityData2[256];
          int curpos=0;
          entityData2[curpos]=0x14; //Named Entity Spawn
          curpos++;
          putSint32(&entityData2[curpos], Users[i].UID);        
          curpos+=4;
          entityData2[curpos]=0;
          entityData2[curpos+1]=Users[i].nick.size();
          curpos+=2;
      
          for(int j=0;j<Users[i].nick.size();j++)
          {
            entityData2[curpos]=Users[i].nick[j];
            curpos++;
          }
      
          putSint32(&entityData2[curpos],Users[i].pos.x*32);
          curpos+=4;
          putSint32(&entityData2[curpos],Users[i].pos.y*32);
          curpos+=4;
          putSint32(&entityData2[curpos],Users[i].pos.z*32);
          curpos+=4;
          entityData2[curpos]=0; //Rotation
          entityData2[curpos+1]=0; //Pitch
          curpos+=2;
          putSint16(&entityData2[curpos],0); //current item
          curpos+=2;
          h.SendSock(this->sock,(uint8 *)&entityData2[0], curpos);
        }
      }

    }

    bool addUser(SOCKET sock,uint32 EID)
    {
        User newuser(sock,EID);
        Users.push_back(newuser);

        return true;
    }

    bool remUser(SOCKET sock)
    {
        unsigned int i;
        for(i=0;i<(int)Users.size();i++)
        {
            if(Users[i].sock==sock)
            {
                Users.erase(Users.begin()+i);
                //Send quit to all
                //h.SendAll(std::string((char *)&data[0], 7));
                //std::cout << "Send remove UID: " << UID << std::endl;
                return true;
            }
        }
        return false;
    }

    bool isUser(SOCKET sock)
    {
        uint8 i;
        for(i=0;i<Users.size();i++)
        {
            if(Users[i].sock==sock)
                return true;
        }
        return false;
    }

    //Generate random and unique entity ID
    uint32 generateEID()
    {
      uint32 EID;
      bool finished=false;
      srand ( time(NULL) );
              
      while(!finished)
      {
        finished=true;
        EID=rand() & 0xffffff;

        for(uint8 i=0;i<Users.size();i++)
        {
          if(Users[i].UID==EID)
          {
            finished=false;
          }
        }
      }
      return EID;
    }


