/**
 * Send email when your Hypernode is down
 * If status is -1 (offline your HN) then its send out a mail that HN is offline.
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
// Your mail
const char *fromAddress = "noreply@bismuth.net";
// Where to send email
const char *toAddress;
// SMTP server
const char *smtpServer;
// SMTP port
int smtpPort;
// SMTP protocol
const char *smtpProtocol;
// SMTP account (email account)
const char *smtpUser;
// SMTP password (mail password)
const char *smtpPasswd;

typedef struct
{
  const char** content;
  size_t pos;
} EmailData;

struct memoryStruct {
  char *memory;
  size_t size;
};
typedef struct memoryStruct jsonData ;


// Print configuration error and exit.
void die(config_setting_t *cfg, const char *fmt, ...)
{
	va_list vargs;
	const char *file = config_setting_source_file(cfg);

	va_start(vargs, fmt);
  vfprintf (stderr, fmt, vargs);
  fprintf (stderr, " in %s\n", file);
	va_end(vargs);

  exit (EXIT_FAILURE);
}

//  Get absolute path (current working directory) on startup application
char *getCwd(char *argv0)
{
  static char abs_exe_path[PATH_MAX];
  char path_save[PATH_MAX];
  char *p;

  if(!(p = strrchr(argv0, '/')))
  {
    if (getcwd(abs_exe_path, sizeof(abs_exe_path)) == NULL) die(NULL, "getCwd(): getcwd() error");
  }
  else
  {
    *p = '\0';
    if (getcwd(path_save, sizeof(path_save)) == NULL) die(NULL, "getCwd(): getcwd() error");
    if (chdir(argv0) != 0) die(NULL, "getCwd(): chdir() error");
    if (getcwd(abs_exe_path, sizeof(abs_exe_path)) == NULL) die(NULL, "getCwd(): getcwd() error");
    if (chdir(path_save) != 0) die(NULL, "getCwd(): chdir() error");
  }

  return abs_exe_path;
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

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) return 0;

  EmailData *upload_ctx = (EmailData*) userp;
  const char *data = upload_ctx->content[upload_ctx->pos];

  if(data) {
    size_t len = strlen(data);
    if (len > size * nmemb) return CURL_READFUNC_ABORT;
    memcpy(ptr, data, len);
    upload_ctx->pos++;

    return len;
  }

  return 0;
}

/*
  Returns a string suitable for RFC 2822 compliant Message-ID, e.g:
  <1536735548.132.901697899@mail.gmail.com>
*/
//   printf("%s\n", makeMsgID("cnhx27@gmail.com"));

char *makeMsgID(char *Adress)
{
  static char messageID[HOST_NAME_MAX];
  unsigned int timeval = (unsigned int)time(NULL);
  int pid = getpid();
  srand(time(NULL)); 
  int randint = rand();

  char *dupAdress = strdup(Adress);
  strsep(&dupAdress, "@");
  char *token = strsep(&dupAdress, "@");

  sprintf(messageID, "%u.%d.%d@mail.%s", timeval, pid, randint, token);

  return messageID;
}

void initEmailData(EmailData* data, const char* to, const char* from, const char* subject, const char* input)
{
  // the added numbers to the length of the string are the pre-computed
  // lengths of the strings to be concatenated + 1 (for termination char)
  size_t htmlSize = strlen(input) + 24;
  char *html = malloc(htmlSize);
  snprintf(html, htmlSize, "%s%s%s", "<div dir=\"ltr\">", input, "</div>\r\n");

  size_t headerSize = strlen(subject) + 12;
  char *headerSubject = malloc(headerSize);
  snprintf(headerSubject, headerSize, "Subject: %s\r\n", subject);

  headerSize = strlen(to) + 7;
  char *headerTo = malloc(headerSize);
  snprintf(headerTo, headerSize, "To: %s\r\n", to);

  headerSize = strlen(from) + 9;
  char *headerFrom = malloc(headerSize);
  snprintf(headerFrom, headerSize, "From: %s\r\n", from);

  headerSize = 40;
  char *headerDate = malloc(headerSize);
  time_t rawtime = 0;
  time(&rawtime);
  struct tm *timeinfo = localtime(&rawtime);
  strftime (headerDate, headerSize, "Date: %a, %d %b %Y %T %z\r\n", timeinfo);

  const char *setup[] =
  {
          // extends the format of email to support more data (vidoes, audio, etc)
          //"MIME-Version: 1.0\r\n",
          // when the email was sent
          headerDate,
          // who the email is to
          headerTo,
          // who the email is from
          headerFrom,
          // email subject, optional
          headerSubject,
          // extends the format of email to support more data (vidoes, audio, etc)
          "MIME-Version: 1.0\r\n",
          // describe the data contained in the body
          // https://tools.ietf.org/html/rfc2046#section-5.1.1
          "Content-Type: multipart/alternative; boundary=border\r\n",
          // empty line to divide headers from body, see RFC5322
          "\r\n",
          // all boundaries must start with two hyphens
          "--border\r\n",
          // let recipient interpret displaying text, send plain text & HTML
          // specify the plain text encoding
          "Content-Type: text/plain; charset=UTF-8\r\n",
          "\r\n",
          input,
          "\r\n",
          "--border\r\n",
          // specify the HTML encoding
          "Content-Type: text/html; charset=UTF-8\r\n",
          "\r\n",
          html,
          "\r\n",
          // final boundary ends with two hyphens as well
          "--border--\r\n",
          "\r\n",
          "\r\n",
          NULL
  };

  int setupSize = sizeof(setup)/sizeof(setup[0]);
  data->content = malloc(sizeof(char*) * setupSize); 
  for(int i=0; setup[i] != NULL; i++)
  {
    printf("setup[%d]: %s", i,setup[i]);
      data->content[i] = setup[i];
  }
  data->pos = 0;
}

void sendEmail(const char* to, const char* from, const char* subject, const char* input)
{
  CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
  EmailData upload_ctx = {0};

  initEmailData(&upload_ctx, toAddress, fromAddress, subject, input);

  curl = curl_easy_init();
  if(curl) 
  {
    char smtpURL[100];
    sprintf(smtpURL, "%s://%s:%d/", smtpProtocol, smtpServer, smtpPort);

    /* This is the URL for your mailserver. Note the use of port 587 here,
     * instead of the normal SMTP port (25). Port 587 is commonly used for
     * secure mail submission (see RFC4403), but you should use whatever
     * matches your server configuration. */
    curl_easy_setopt(curl, CURLOPT_URL, smtpURL);

    /* we'll start with a plain text connection, and upgrade
     * to Transport Layer Security (TLS) using the STARTTLS command. 
     */ 
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  

    // Set username and password
    curl_easy_setopt(curl, CURLOPT_USERNAME, smtpUser);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, smtpPasswd);

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, fromAddress);   
    recipients = curl_slist_append(recipients, toAddress);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    
    // callback function to specify the payload (the headers and body of the message)
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx); 
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    #ifdef DEBUG    
      /* Since the traffic will be encrypted, it is very useful to turn on debug
      * information within libcurl to see what is happening during the transfer.
      */ 
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    #endif

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "Send mail FAILED: %s (STMP server: %s)\n", curl_easy_strerror(res), smtpURL);
    else
      printf("Send mail OK (STMP server: %s)\n", smtpURL);

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);    
  }
}

void getJSON(jsonData *chunk)
{
  CURL *curl;
	CURLcode res = CURLE_OK;

  // Libcurl Get https://curl.haxx.se/libcurl/c/getinmemory.html
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

void supervisioHN(int signum) 
{
  jsonData data;
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
          time_t now;
          struct tm *tm;

          time(&now);
          tm = localtime(&now);
          strftime(&buffTime[0], 32, "%a, %d %b %Y %H:%M:%S %z", tm);

          int StatusHN = atoi(((struct json_number_s *)IpAddressHN->value->payload)->number);
          if (StatusHN <= 0)
          {
            char message[100];
            printf("%s, %s [ OFFLINE ], send email to %s\n", buffTime, IpAddressHN->name->string, toAddress);
            snprintf(message, sizeof(message), "Hypernode IP: %s OFF Line Status: %d", IpAddressHN->name->string, StatusHN);

            sendEmail(toAddress, fromAddress, "BISMUTH NODE ALERT", message);

            break;
          } else 
          {
            printf("%s, %s [ ONLINE ]\n", buffTime, IpAddressHN->name->string);
          }
        }
      }
      IpAddressHN = IpAddressHN->next;
    }

    free(data.memory);
    free(jsonRootHN);
  }

  alarm(timeSupervision);
}

int main(int argc, char **argv)
{
  config_t cfg;
  char cfgFileName[PATH_MAX + NAME_MAX];
  snprintf(cfgFileName, PATH_MAX + NAME_MAX, "%s/%s", getCwd(argv[0]), "HNAnative.cfg");

  config_init(&cfg);
  // Read the config file
  if(!config_read_file(&cfg, cfgFileName))
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
  // Where to send email
  if (!config_setting_lookup_string(cfgSetting, "toAddress", &toAddress))
		die(cfgSetting, "No 'toAddress' setting");
  // SMTP server
  if (!config_setting_lookup_string(cfgSetting, "smtpServer", &smtpServer))
    die(cfgSetting, "No 'smtpServer' setting");
  // SMTP port (537 or 465)
  if (!config_setting_lookup_int(cfgSetting, "smtpPort", &smtpPort))
    die(cfgSetting, "No 'smtpPort' setting");
  // SMTP protocol (smtp or smtps)
  if (!config_setting_lookup_string(cfgSetting, "smtpProtocol", &smtpProtocol))
    die(cfgSetting, "No 'smtpProtocol' setting");
  // SMTP account (email account)
  if (!config_setting_lookup_string(cfgSetting, "smtpUser", &smtpUser))
    die(cfgSetting, "No 'smtpUser' setting");
  // SMTP password (mail password)
  if (!config_setting_lookup_string(cfgSetting, "smtpPasswd", &smtpPasswd))
    die(cfgSetting, "No 'smtpPasswd' setting");

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

  return(EXIT_SUCCESS);
}
