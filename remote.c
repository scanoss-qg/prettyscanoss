#include <stdio.h>
#include <stdlib.h>
#include <string.h>



  #define MAXLINE 500


#define BUFSIZE 128
typedef struct IndexedChar
{ 
  int line;
  char ch;
} IndexedChar;

void process_match(json_object_entry value)
{
  int array_length = value.value->u.array.length;
  //match_data_list *list = calloc(1, sizeof(match_data));
  //list->match_list = calloc(1, sizeof(match_data));
  //list->count = array_length;

  for (int i = 0; i < array_length; i++)
  {
    match_data *match = calloc(1, sizeof(match_data));
    json_object_entry *match_value = value.value->u.array.values[i]->u.object.values;
    int match_obj_len = value.value->u.array.values[i]->u.object.length;

   // list->match_list[i] = calloc(1, sizeof(match_data)); //we dont need a match list for the moment
    match_data * new_item = calloc(1, sizeof(match_data));
    for (int j = 0; j < match_obj_len; j++)
    {
      if (!strcmp(match_value[j].name, "id"))
      {
        if (strstr(match_value[j].value->u.string.ptr, "none"))
          break;
        strcpy(match->idtype, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "vendor"))
      {
        strcpy(new_item->vendor, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "component"))
      {
        strcpy(new_item->component, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "version"))
      {
        strcpy(new_item->version, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "latest"))
      {
        strcpy(new_item->latest_version, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "url"))
      {
        strcpy(new_item->url, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "release_date"))
      {
         strcpy(new_item->release_date, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "file"))
      {
         strcpy(new_item->filename, match_value[j].value->u.string.ptr);
      } 
      if (!strcmp(match_value[j].name, "purl"))
      {
        strcpy(new_item->purl, match_value[j].value->u.array.values[0]->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "licenses"))
      {
        if (match_value[j].value->u.array.length > 0)
        {
          strcpy(new_item->license, match_value[j].value->u.array.values[0]->u.object.values->value->u.string.ptr);
        }
      }   
      /*if (!strcmp(match_value[j].name, "lines"))
      {
         strcpy(match->lines, match_value[j].value->u.string.ptr);
      }*/
      /*if (!strcmp(match_value[j].name, "oss_lines"))
      {
         strcpy(match->oss_lines, match_value[j].value->u.string.ptr);
      }*/ //<---- was commented due to a bug on mac.
      if (!strcmp(match_value[j].name, "matched"))
      {
         strcpy(match->matched, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "size"))
      {
        strcpy(match->size, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "url_hash"))
      {
        strcpy(match->md5_comp, match_value[j].value->u.string.ptr);
       }
      if (!strcmp(match_value[j].name, "file"))
      {
        strcpy(match->filename, match_value[j].value->u.string.ptr);
      }
      if (!strcmp(match_value[j].name, "licenses"))
      {
        if (match_value[j].value->u.array.length > 0)
        {
           strcpy(match->license, match_value[j].value->u.array.values[0]->u.object.values->value->u.string.ptr);
         }
      }
    }
    add_component(new_item);
    free(match);
    free(new_item);
    //list->match_list[i] = new_item;
  }
 // match_list_free(list);
}

/* Output contents of component_list in the requested format */
void print_matches(FILE * output, char * format)
{
	bool cyclonedx = false;

  if (strstr(format,SCANNER_FORMAT_CYCLONEDX))
    cyclonedx = true;
    
  if (cyclonedx)
    cyclonedx_open(output);
  else
    spdx_open(output);

  for (int i = 0; i < CRC_LIST_LEN && *component_list[i].purl; i++)
	{
		if (i) 
      fprintf(output,"  ,\n");
	  
    if (cyclonedx) 
      print_json_match_cyclonedx(output, &component_list[i]);
  	else 
      print_json_match_spdx(output, &component_list[i]);
	}

   if (cyclonedx)
    cyclonedx_close(output);
  else
    spdx_close(output);

}
int normalize(char *text,int maxLen, IndexedChar *tempText){

  int currentIndex = 0;
  int line =1;
  for(int i=0;i<maxLen-1;i++){
       if(text[i]=='\n') {line++ ;continue;}
        if((text[i]=='\r') || (text[i]=='\n') || (text[i]==' ') || (text[i]=='\t')) continue;
        tempText[currentIndex].ch=text[i];
        tempText[currentIndex].line=line;
        currentIndex ++;
  }
 tempText[currentIndex].line=-1;
   currentIndex ++;
 
  return currentIndex;
}


/* Returns the current date stamp */
char *datestamp(void)
{
	time_t timestamp;
	struct tm *times;
	time(&timestamp);
	times = localtime(&timestamp);
	char *stamp = malloc(MAX_ARGLN);
	strftime(stamp, MAX_ARGLN, "%FT%T%z", times);
	return stamp;
}

void compare(char *local,int localLen, char *remote,int remoteLen){

//matchesLocal=[]
//matchesRemote=[]
 IndexedChar localIndex[200000];
 IndexedChar remoteIndex[200000];
 int lenLocal = normalize(local,localLen,localIndex);
 int lenRemote = normalize(remote,remoteLen,remoteIndex);
 int j =0;
 int i =0;
 int lastRemote = 0;


    for(i=0;i<lenLocal-1;i++){
       
        for(j=0;j<lenRemote-1;j++){
            int curr_remote =j;
            int curr_local =i;
            int lastLocal =i;
            lastRemote=j;
            while (1){
                if(curr_remote<lenRemote-1 && curr_local<lenLocal && localIndex[curr_local].ch==remoteIndex[curr_remote].ch){
                lastLocal=curr_local;
                lastRemote= curr_remote;
                curr_remote++;
                curr_local++;
              } else {
                curr_local--;
                curr_remote --;
                break;
            }
        }
            if((curr_remote-j)>25) {
             /*
               matchesLocal.push(textA[i].line+"-"+textA[lastLocal].line)
               matchesRemote.push(textB[j].line+"-"+textB[lastRemote].line)
             */
            printf("Local %d - %d Remoto %d - %d\n",localIndex[i].line,localIndex[lastLocal].line, remoteIndex[j].line,remoteIndex[lastRemote].line);
               i=curr_local;
            }
        }
    }/*
    localMatchStr="";
    remoteMatchStr=""
    if(matchesLocal.length>0){
        let k=0
        for( k=0;k<matchesLocal.length-1;k++){
            localMatchStr+=matchesLocal[k]+","
            remoteMatchStr+=matchesRemote[k]+","
        }
        localMatchStr+=matchesLocal[k]
        remoteMatchStr+=matchesRemote[k]
return ({local:localMatchStr, remote:remoteMatchStr})
    }


}*/


}

void main(void){
  FILE *f = fopen("main.c","r");
  fseek(f, 0L, SEEK_END);
  int sz = ftell(f);
  rewind(f);
  char fileContent[100000];
  fread(fileContent,1,sz,f);
  compare(fileContent,sz,fileContent,sz);

 fclose(f);

}






 void mainold(void) {
  //  char *cmd = "scanner /home/oscar/scanoss/minr/src/minr.c ";    
//    char *cmd ="scanner  /home/oscar/scanoss/minr/src/minr.c  |
//   char *cmd=" cat  result.json |jq -r 'to_entries[] |select(.value[]|.id=null)| \"\\(.value[0]|.id ) \\(.value[0]|.oss_lines) \\(.value[0]|.purl[0] ) \\(.value[0]|.file ) \\(.value[0]|.url )\"'";
char *cmd ="cat  result.json |jq -r 'to_entries[] |select(.value[]|.id!=null)| \"\\(.value[0]|.copyrights[0]|.name )\"'";
  
  
  
    
    
FILE *fp;
    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }   
char buf[100];  
char licenses[10000];
char purl[500];
char id[100];
char oss_lines[1000];
char filePath[1000];
char url[1000];
    while (fgets(buf, BUFSIZE, fp) != NULL) {
    if(!(strstr(buf,"null")))
    printf("->%s\n",buf);
    //	sscanf(buf,"%s %s %s %s %s",id,oss_lines,purl,filePath,url);
    
    	//printf("Match type: %s\nMatched lines %s\nPURL %s\nPath %s\nurl%s\n", id,oss_lines,purl,filePath,url);
    }

   /* if (pclose(fp)) {
        printf("Command not found or exited with error status\n");
        return -1;
    }*/

    return 0;
}








  void mainb(int argc, char *argv[])
  {
     char linebuf[MAXLINE];
     unsigned char output[10000];
     memset(output,0,10000);
     char *cmdbuf;
     int cmdlen;
     FILE *sort_output;
     int i=0;
     
     
      FILE *f;
  f = fopen("/home/oscar/scanoss/minr/src/main.c","r");
	fseek(f, 0, SEEK_END);
  int size = ftell(f);
  unsigned char *contents = calloc(1, size);
  memset(contents, '\0', size);
  rewind(f);
 
  size_t readSize = fread(contents, size, 1, f);

                   
    // asprintf(&cmdbuf,"scanner -s quique ");
       
//asprintf(&cmdbuf,"scanner -s quique ");
       asprintf(&cmdbuf,"ls");

     sort_output = popen(cmdbuf, "w");  /* Read the output of sort.*/
     if (!sort_output) {
        perror("popen failure");
        exit(EXIT_FAILURE);
     }
//    fprintf(sort_output,"%s",contents);
    //fputs(contents, sort_output);
//     fwrite(contents,size,1,sort_output);
  //  fwrite("%c",EOF,1,sort_output);
  fputc(EOF, sort_output);
     fflush(sort_output);
   /*while(!(feof(sort_output) || ferror(sort_output))){
			fscanf(sort_output,"%s",&output[i]);
			i=strlen(output);
     */     
  // fflush(sort_output); 
   
     i=0; 
    
    while (fgets(output,10, sort_output) != NULL) {
        // Do whatever you want here...
        printf("OUTPUT: %s", output);
    }
    pclose(sort_output);  

//printf("%s",output);
    /* if (feof(sort_output) || ferror(sort_output)) {
       
        fputs("Input error.n",sort_output);
        pclose(sort_output); 
        exit(EXIT_FAILURE);
     }*/
             /* Write line to stdout.             */

        /* Close the sort process. It will probably terminate      */
        /*  with SIGPIPE.                                          */



  
                    
     exit(EXIT_SUCCESS);
  }

int maino (void)
{
  FILE *output;
  FILE *f;
  f = fopen("/home/oscar/scanoss/scanner.c/src/scanner.c","r");
	fseek(f, 0, SEEK_END);
  int size = ftell(f);
  unsigned char *contents = calloc(1, size);
  memset(contents, '\0', size);
  rewind(f);
 
  size_t readSize = fread(contents, size, 1, f);

	


  //output = popen ("scanner -s qq ", "w");
  output = popen ("ls ", "w");
  if (!output)
    {
      fprintf (stderr,
               "incorrect parameters or too many files.\n");
      return EXIT_FAILURE;
    }
    
//fprintf("%s","hola mundo\n",output);

int i=0;
if(feof(stdout)) printf("EOF\n");
else {
while(!feof(stdout)){
char c =fgetc(stdout);
i++;
}
}
printf("%d\n",i);
  if (pclose (output) != 0)
    {
      fprintf (stderr,
               "Could not run  or other error.\n");
    } 
  
    
  return EXIT_SUCCESS;
}

