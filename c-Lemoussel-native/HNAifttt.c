/**
 * Check your HN status with IFTT Webhooks
 * If status is -1 (offline your HN) then its trigger IFTT webhooks event.
 * JSON Hypernode status list: https://hypernodes.bismuth.live/status.json. Updated every 3 minutes.
 *
 * Requirements:
 * json-h  - https://github.com/sheredom/json.h
 *        include in project
 * libcurl - http://curl.haxx.se/libcurl/c
 *        sudo apt install libcurl4-gnutls-dev
 * libconfig - https://github.com/hyperrealm/libconfig
 *        sudo apt install libconfig-dev
 * 
 * Build:
 * see Makefile 
 *
 * Run:
 * ./HNAnative
 * 
 */

#define _GNU_SOURCE

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>

// https://github.com/hyperrealm/libconfig
#include <libconfig.h>
// http://curl.haxx.se/libcurl/c
#include <curl/curl.h>
// https://github.com/sheredom/json.h
#include "json_h/json.h"

// how often to check your HN status, minimum 180 sec (3 minutes)
int timeSupervision;
// IP of HN
const char **nodeIpAddress;
int nodeIPNumber;
// IFTT Key
const char *iftttKey;
// IFFT Webhook Event
const char **iftttEvent;
int iftttEventNumber;

struct memoryStruct {
  char *memory;
  size_t size;
};
typedef struct memoryStruct curlData;

void getDateTime(char *buffTime)
{
  time_t now;
  struct tm *tm;

  time(&now);
  tm = localtime(&now);
  strftime(&buffTime[0], 32, "%a, %d %b %Y %H:%M:%S %z", tm);  
}

void die(config_setting_t *cfg, const char *fmt, ...)
{
  const char *cfgFile;

  va_list vargs;
  if (cfg != NULL) cfgFile = config_setting_source_file(cfg);

	va_start(vargs, fmt);
  vfprintf (stderr, fmt, vargs);
  if (cfg != NULL) fprintf (stderr, " in %s", cfgFile);
  fprintf(stderr, "\n");
  va_end(vargs);

  exit(EXIT_FAILURE);
}


static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct memoryStruct *mem = (struct memoryStruct *)userp;
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    // out of memory! 
    fprintf(stderr, "not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

void getJSON(curlData *chunk)
{
  CURL *curl;
	CURLcode res = CURLE_OK;

  chunk->memory = malloc(1);
  chunk->size = 0;

  curl = curl_easy_init();
  if(curl) 
  {
    curl_easy_setopt(curl, CURLOPT_URL, "https://hypernodes.bismuth.live/status.json");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  
    res = curl_easy_perform(curl);
 
    if(res != CURLE_OK) fprintf(stderr, "Get JSON data failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
  }
}

void sendIFTT_event(const char *iftttEvent, const char *iftttKey, const char*ipHN, int statusHN)
{
  CURL *curl = curl_easy_init();
  if(curl) {
    char errbuf[CURL_ERROR_SIZE] = { 0, };
    char curlAgent[1024] = { 0, };
    char iftttUrl[2000] = { 0, };
    struct memoryStruct curlBuffer;

    curlBuffer.memory = malloc(1);
    curlBuffer.size = 0;

    snprintf(curlAgent, sizeof curlAgent, "libcurl/%s", curl_version_info(CURLVERSION_NOW)->version);
    curlAgent[sizeof curlAgent - 1] = 0;
    curl_easy_setopt(curl, CURLOPT_USERAGENT, curlAgent);

    struct curl_slist *headers =NULL;
    headers = curl_slist_append(headers, "Expect:");    
    headers = curl_slist_append(headers , "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // IFTT webhooks: https://ifttt.com/maker_webhooks
    snprintf(iftttUrl, sizeof iftttUrl, "https://maker.ifttt.com/trigger/%s/with/key/%s", iftttEvent, iftttKey);
    curl_easy_setopt(curl, CURLOPT_URL, iftttUrl);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{ \"value1\" : \"192.99.248.44\", \"value2\" : -1 }");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);    
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlBuffer);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) die(NULL, "curl post to IFTTT webhooks failed! Error: %s\n%s", curl_easy_strerror(res), errbuf);

    char buffTime[32];
    getDateTime(buffTime);
    printf("%s, IFTTT: %s\n", buffTime, curlBuffer.memory);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

void supervisioHN(int signum) 
{
  curlData data;
  getJSON(&data);

  if (data.size > 0) 
  {
    struct json_value_s* jsonRootHN = json_parse(data.memory, data.size);
    struct json_object_s* objectHN = (struct json_object_s *)jsonRootHN->payload;
    struct json_object_element_s* IpAddressHN = objectHN->start;
  
    for (int i = 0; i < objectHN->length; i++) 
    {
      for (int j = 0; j < nodeIPNumber; j++) 
      {
        if (strcmp(IpAddressHN->name->string, nodeIpAddress[j]) == 0)
        {
          char buffTime[32];
          getDateTime(buffTime);
   
          int StatusHN = atoi(((struct json_number_s *)IpAddressHN->value->payload)->number);
          if (StatusHN <= 0)
          {
            printf("%s, %s [ OFFLINE ]\n", buffTime, IpAddressHN->name->string);
            for (int k = 0; k < iftttEventNumber; k++) 
            {
              sendIFTT_event(iftttEvent[k], iftttKey, IpAddressHN->name->string, StatusHN);
            }
            break;
          } else {
            printf("%s, %s [ ONLINE ]\n", buffTime, IpAddressHN->name->string);
          }
        }
      }
      IpAddressHN = IpAddressHN->next;
    }

    free(data.memory);
  }

  alarm(timeSupervision);
}

int main(void)
{
  config_t cfg;

  config_init(&cfg);
  // Read the config file
  if(!config_read_file(&cfg, "HNAifttt.cfg"))
  {
    fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    return(EXIT_FAILURE);
  }
  config_setting_t *cfgSetting = config_root_setting(&cfg);

  // How often to check your HN status, minimum 180 sec (3 minutes).
  if (!config_setting_lookup_int(cfgSetting, "timeSupervision", &timeSupervision))
		die(cfgSetting, "No 'timeSupervision' setting");  
  // IP of Hypernode
  config_setting_t *cfg_NodeIP = config_lookup(&cfg, "nodeIpAddress");
  if (cfg_NodeIP) 
  {
    if (!config_setting_is_array(cfg_NodeIP))
    		die(cfgSetting, "'nodeIpAddress' parameters must be an array of IP of Hypernode!");
    nodeIPNumber = config_setting_length(cfg_NodeIP);
    nodeIpAddress = malloc(nodeIPNumber * sizeof(char) * strlen("255.255.255.255"));
	  for (int i = 0; i < nodeIPNumber; i++) nodeIpAddress[i] = config_setting_get_string_elem(cfg_NodeIP, i);
  } else die(cfgSetting, "No 'nodeIpAddress' setting");
  // IFTT Key
  if (!config_setting_lookup_string(cfgSetting, "iftttKey", &iftttKey))
		die(cfgSetting, "No 'iftttKey' setting");  
  // IFFT Webhook Event
  config_setting_t *cfg_IftttEvent = config_lookup(&cfg, "iftttEvent");
  if (cfg_IftttEvent) 
  {
    if (!config_setting_is_array(cfg_IftttEvent))
    		die(cfgSetting, "'iftttEvent' parameters must be an array of IP of Hypernode!");    
    iftttEventNumber = config_setting_length(cfg_IftttEvent);
    iftttEvent = malloc(iftttEventNumber * sizeof(char) * 20); // 20 = max. length IFTT Webhook event name
	  for (int i = 0; i < iftttEventNumber; i++) iftttEvent[i] = config_setting_get_string_elem(cfg_IftttEvent, i);
  } else die(cfgSetting, "No 'iftttEvent' setting");

  curl_global_init(CURL_GLOBAL_ALL);
  if(atexit(curl_global_cleanup)) {
    curl_global_cleanup();
    die(NULL, "atexit() failed to register curl_global_cleanup");
  }

  signal(SIGALRM, supervisioHN);
  alarm(timeSupervision);

  // Wait forever!
  // Sleep until SIGALRM arrives. This blocks every signal except SIGALRM.
  sigset_t mysigset;
  sigfillset(&mysigset);
  sigdelset(&mysigset, SIGALRM);
  while (1)
  {
    sigsuspend(&mysigset);
  }

  return EXIT_SUCCESS;
}
