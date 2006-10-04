/***************************************************************************
                          wanrouter_rc_file_reader.cpp  -  description
                             -------------------
    begin                : Wed Mar 24 2004
    copyright            : (C) 2004 by David Rokhvarg
    email                : davidr@sangoma.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wanrouter_rc_file_reader.h"
#include "string.h" //for memset()

#define DBG_WANROUTER_RC_FILE_READER 1

wanrouter_rc_file_reader::wanrouter_rc_file_reader(int wanpipe_number)
{
  Debug(DBG_WANROUTER_RC_FILE_READER,
    ("wanrouter_rc_file_reader::wanrouter_rc_file_reader()\n"));
   
  wanpipe_name.sprintf("wanpipe%d", wanpipe_number);

  full_file_path = wanpipe_cfg_dir;
  full_file_path += "wanrouter.rc";

  Debug(DBG_WANROUTER_RC_FILE_READER, ("full_file_path: %s\n", (char*)full_file_path.c_str()));
}

wanrouter_rc_file_reader::~wanrouter_rc_file_reader()
{
  Debug(DBG_WANROUTER_RC_FILE_READER,
    ("wanrouter_rc_file_reader::~wanrouter_rc_file_reader()\n"));
}

//returns:  YES - device name found
//          NO  - device name not found or failed to parse the file
//
//          if 'update_flag' is yes and device is NOT found, device will be added
int wanrouter_rc_file_reader::search_boot_start_device()
{
  int rc = YES;
  FILE* wanrouter_rc_file = NULL;
  char tmp_read_buffer[MAX_PATH_LENGTH];
  char tmp_read_buffer1[MAX_PATH_LENGTH];
  char tmp_read_buffer2[MAX_PATH_LENGTH];
  char *tmp, *tmp1=NULL;
  string str_tmp1;

  updated_wandevices_str = "";
  updated_file_content = "";
	  
  if(check_file_exist((char*)full_file_path.c_str(), &wanrouter_rc_file) == NO){
    Debug(DBG_WANROUTER_RC_FILE_READER, ("The 'Net interface' file '%s' does not exist!\n",
      (char*)full_file_path.c_str()));
    return NO;
  }

  wanrouter_rc_file = fopen((char*)full_file_path.c_str(), "r+");
  if(wanrouter_rc_file == NULL){
    ERR_DBG_OUT(("Failed to open the 'Net interface' file '%s' for reading!\n",
      (char*)full_file_path.c_str()));
    return NO;
  }

  do{
    fgets(tmp_read_buffer, MAX_PATH_LENGTH, wanrouter_rc_file);

    if(!feof(wanrouter_rc_file)){
      //must have an original copy, because strstr() changing the original buffer!!!
      memcpy(tmp_read_buffer1, tmp_read_buffer, MAX_PATH_LENGTH);

      tmp = NULL;
      if(tmp_read_buffer[0] != '#'){
        tmp = strstr(tmp_read_buffer, "WAN_DEVICES=");
      }
      if(tmp != NULL){
        
        updated_wandevices_str = tmp_read_buffer1;
	
	tmp += strlen("WAN_DEVICES=");//skip past WAN_DEVICES=
	tmp1 = strstr(tmp, wanpipe_name.c_str());
	if(tmp1 != NULL){
	  //in the list already
	}else{
	  //not in the list
	  str_tmp1 = " ";
	  str_tmp1 += wanpipe_name;
	  str_tmp1 += "\"";
	  wanpipe_name = str_tmp1;

	  //get rid of the closing <"> quotaion mark
          memcpy(tmp_read_buffer2, tmp, strlen(tmp) - 2);
	  //append the new wanpipe name
	  str_tmp1 = tmp_read_buffer2;
	  str_tmp1 += wanpipe_name;
	  //prepend "WAN_DEVICES="
	  wanpipe_name = str_tmp1;
	  str_tmp1 = "WAN_DEVICES=";
	  updated_wandevices_str = (str_tmp1 += wanpipe_name);
	  updated_wandevices_str += "\n";
	     
	  Debug(DBG_WANROUTER_RC_FILE_READER, ("tmp: %s\n", tmp));
	  Debug(DBG_WANROUTER_RC_FILE_READER, ("tmp_read_buffer2: %s\n", tmp_read_buffer2));
	  Debug(DBG_WANROUTER_RC_FILE_READER, ("updated_wandevices_str: %s\n", updated_wandevices_str.c_str()));
	}

	updated_file_content += updated_wandevices_str;
      }else{
	updated_file_content += tmp_read_buffer1;
      }   
      
    }//if()
    
  }while(!feof(wanrouter_rc_file));

  fclose(wanrouter_rc_file);

  Debug(DBG_WANROUTER_RC_FILE_READER, ("updated_file_content: %s\n", updated_file_content.c_str()));

  if(tmp1 != NULL){
    rc = YES;
   }else{
    rc = NO;
  }
  return rc;
}

int wanrouter_rc_file_reader::update_wanrouter_rc_file()
{
  return write_string_to_file((char*)full_file_path.c_str(),
		 (char*)updated_file_content.c_str());
}

