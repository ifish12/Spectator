//
//  main.cpp
//
//
//  This is a small C++ file that pulls data from Riot Games' current games API.
//  The user inputs the summoner name and then does API magic to get the parameters
//  required to call League of Legends via the commandline to enter spectater mode
//  without the need of ever opening the client itself. It saves lots of time
//
//
//
//  Created by Geoff Shapiro on 21/02/15
//  Copyright (c) 2015 Geoff Shapiro. All rights reserved.
//
#include <iostream>
#include <string>
#include <cstring>
#include <curl/curl.h> // doesn't work in windows, consider moving to boost or some other library in the future
#include "json/json.h"
#include "key.h"
#include <fstream>
#include <stdio.h>
#include <cstdlib>


using namespace std;

CURL* curl; //our curl object
Json::Value root;
Json::Reader reader;

string sID;
string data; //will hold the url's contents
char name[25] = "";

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up)
{   //callback must have this declaration
    //buf is a pointer to the data that curl has for us
    //size*nmemb is the size of the buffer
    
    for (int c = 0; c<size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }
    return size*nmemb; //tell curl how many bytes we handled
}

int main() {
    
    char dURL1[100] = "https://na.api.pvp.net/api/lol/na/v1.4/summoner/by-name/";
    char dURL2[100] = "?api_key=";
    char dURL3[100] = API_KEY;
    cout << "Who do you wish to spectate, summoner?" << endl;
    cin >> name;
 
    char fURL[225];
    strcat(fURL, dURL1);
    strcat(fURL, name);
    strcat(fURL, dURL2);
    strcat(fURL, dURL3);
    
    curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
    curl = curl_easy_init();
    
    curl_easy_setopt(curl, CURLOPT_URL,fURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    
    curl_easy_perform(curl);
      
    // Let's parse it
    bool parsedSuccess = reader.parse(data, root, false);
    if(parsedSuccess)
    {
        root = root[root.getMemberNames()[0]];
        cout << root["id"];
        
        data = " ";
        cin.get();
        // preparing to build the URL.
        char cmURL1[100] = "https://na.api.pvp.net/observer-mode/rest/consumer/getSpectatorGameInfo/NA1/";
        int  cmURL2i = root["id"].asInt();
        char cmURL2[100];
        snprintf(cmURL2, 100, "%d", cmURL2i);
        char cmURL3[100] = "?api_key=";
        char cmURL4[100] = API_KEY;
        char cmfURL[500];
        // Probably could use snprintf to create the entire URL
        //This is how I build the URLs. It's probably not the best way
        strcat(cmfURL, cmURL1);
        strcat(cmfURL, cmURL2);
        strcat(cmfURL, cmURL3);
        strcat(cmfURL, cmURL4);
        //cout << cmfURL << endl;
        
        curl_easy_setopt(curl, CURLOPT_URL,cmfURL);  // Preparing our CURL object
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
        curl_easy_perform(curl); // Sending the request
        parsedSuccess = reader.parse(data, root, false); // Check to see if we parsed JSON correctly
        if(parsedSuccess) {
            int gameId = root["gameId"].asInt();
            const char *encrypy = root["observers"]["encryptionKey"].asCString();
            
            //cout << gameId << endl;
            //cout << encrypy;
            
            char result[9999];
            char result2[9999];
            sprintf(result, "cd /Applications/League\\ of\\ Legends.app/Contents/LoL/RADS/solutions/lol_game_client_sln/releases/0.0.0.157/deploy/LeagueOfLegends.app/Contents/MacOS" );
            sprintf(result2, "riot_launched=true ./LeagueOfLegends 8394 LoLLauncher \"\" \"spectator spectator.na.lol.riotgames.com:80 %s %d NA1\"", encrypy, gameId);
            
            ofstream outfile ("spectate.sh");
            outfile << result << std::endl;
            outfile << result2 << std::endl;
            outfile.close();
            
            cout << result;
            system("chmod 755 spectate.sh");
            system("./spectate.sh");
        }
        else {
            cout << "Summoner isn't in game right now\n";
        }
    }
    else {
        cout << "user doesn't exist\n";
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    return 0;
}


