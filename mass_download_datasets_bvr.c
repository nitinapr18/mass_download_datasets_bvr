#include<conio.h>
#include<stdio.h>
#include<tc\tc.h>
#include<tccore\item.h>
#include<bom\bom.h>
#include<string.h>
#include<tc\emh.h>
#include<tccore\workspaceobject.h>
#include<fclasses\tc_string.h>
#include<stdlib.h>
#include<tie\tie.h>
#include<cfm\cfm.h>
#include<tccore\grm.h>
#include<ae\ae.h>
#include<ae\dataset.h>
#include<tccore\aom.h>
#include<tccore\aom_prop.h>
#include<sa\imanfile.h>
#include <direct.h>
#include <time.h>

int bom_sub_child(tag_t * tBomChildren,int iNumberOfchild);
void export_dataset(tag_t tItem);
void export_latest_rel_data(tag_t tDocument);
char* get_folder_name(char* cItem_id);
int create_export_folder(char*);
void write_csv_file(tag_t,boolean);
void write_csv_file2(tag_t);

FILE *pFile=NULL;
FILE *pFile2=NULL;
char *cFolderPath= NULL;
int CHECK_ERROR(int a)
{
	int ifail=0;
	char* cError=NULL;
	ifail=a;
	if (ifail==ITK_ok)
	{

	}
	else
	{
		EMH_ask_error_text(ifail,&cError);
		printf("%s",cError);
		exit(0);
	}
	return ifail;
}


int ITK_user_main(int argc,char *argv[]) 
{
	int iFail=ITK_ok;
	int i=0,j=0,k=0;
	int iRevCount=0;
	int iNumberOfchild=0;
	char *cError=NULL;
	tag_t tItem=NULLTAG;
	tag_t *tRevList=NULLTAG;	
	tag_t tWindow=NULLTAG;
	tag_t tBomLine=NULLTAG;
	tag_t *tBomChildren=NULLTAG;
	tag_t tRule=NULLTAG;
	char* cFolderName = NULL;
	int iAttribute =0;
	tag_t tTop_Asm = NULLTAG;
	char* rev_id = NULL;
	char cExportLog[100];
	char cNotExportLog[100];
	char* cName =NULL;
	char *u= ITK_ask_cli_argument ("-u=");
	char *p= ITK_ask_cli_argument ("-p=");
	char *g= ITK_ask_cli_argument ("-g=");
	char *cItem_id=ITK_ask_cli_argument("-item_id=");
	char *cRevRule=ITK_ask_cli_argument("-rev_rule=");
	//char cRevRule[20]=ITK_ask_cli_argument("-rev_rule=");
	int iCheckFolderCreation = 0;
	cFolderPath=ITK_ask_cli_argument("-folder_path=");
	
	if(argc==7)
	{

	}
	else
	{
		printf("\n\n\t\t Please enter correct arguments.. ");
		printf("\n\n\t\t e.g mass_download_datasets_bvr.exe -u=infodba -p=infodba -g=dba -item_id=001 -rev_rule=Any Status; No Working -folder_path=E:\Files");
		return ITK_ok;
	}

	if(cItem_id!=NULL)
	{
		iFail=ITK_init_module(u,p,g);
		if (iFail==ITK_ok)
		{
			printf("\n\n\t\t Login Successful");
			printf("\n\n\t\t Trying to find Item %s",cItem_id);
			iFail=ITEM_find_item(cItem_id,&tItem);

			if((iFail==ITK_ok)&&(tItem!=NULLTAG))
			{
				
				//cFolderPath = tc_strcat(cFolderPath,cName);
				iCheckFolderCreation = create_export_folder(cFolderPath);
				cName = get_folder_name(cItem_id);
				cFolderPath = tc_strcat(cFolderPath,cName);
				create_export_folder(cFolderPath);
				//printf("\t\t\n\n Error : Please enter proper folder path e.g E:\\Files \n");
				//return ITK_ok;
				//cName = get_folder_name(cItem_id,cRevId);
				//cExportLog=strcat(cFolderPath,"\dataset_exported_log.csv");
				//cNotExportLog=strcat(cFolderPath,"\dataset_not_exported_log.csv");
				//pFile=fopen(cFilename,"w+");
				tc_strcpy(cExportLog," ");
				tc_strcpy(cExportLog, cFolderPath);
				tc_strcat(cExportLog,"\\dataset_exported_log.csv");
				
				tc_strcpy(cNotExportLog," ");
				tc_strcpy(cNotExportLog, cFolderPath);
				tc_strcat(cNotExportLog,"\\dataset_not_exported_log.csv");
				
				pFile=fopen(cExportLog,"w+");
				pFile2=fopen(cNotExportLog,"w+");
				
				fprintf(pFile,"\n%s,%s,%s,%s,%s,%s,%s,%s,%s",
					"Item ID","Rev","Name","Status","Action","Lastest Rev Avaialble","Rev","Status","Action");

				fprintf(pFile2,"\n%s,%s,%s,%s,%s",
					"Item ID","Rev","Name","Status","Reason");

				//CHECK_ERROR(ITEM_ask_rev_name(tItem,cRevName));
				//printf("\n\n\t\t Revision ID = %s \t Name = %s",cRevId,cRevName);

				iFail=BOM_create_window(&tWindow);
				printf("\n\n\t\t Finding Revision Rule %s ",cRevRule);
				//cRevRule = "Any Status; No Working";
				//iFail = CFM_find(cRevRule,&tRule);
				if(tc_strcmp(cRevRule,"Any Status; No Working")==0 || tc_strcmp(cRevRule,"Any Status; Working")==0 ||tc_strcmp(cRevRule,"Latest Working")==0)
				{
					if(tc_strcmp(cRevRule,"Any Status; No Working")==0)
						CFM_find("Any Status; No Working",&tRule);
					if(tc_strcmp(cRevRule,"Any Status; Working")==0)
						CFM_find("Any Status; Working",&tRule);
					if(tc_strcmp(cRevRule,"Latest Working")==0)
						CFM_find("Latest Working",&tRule);
				}
				else
				{
					
					printf("\n\n Please Enter correct revision rule Error");
					return ITK_ok;
				}
				//iFail = CFM_find("Any Status; No Working",&tRule);
				//if(iFail != ITK_ok)
				//{
					CHECK_ERROR(BOM_set_window_config_rule( tWindow, tRule )); 
					printf("\n\n\t\t Revision Rule Set ");
					BOM_save_window(tWindow);

					iFail=BOM_set_window_top_line(tWindow,tItem,NULLTAG,NULLTAG,&tBomLine);
					iFail=BOM_line_ask_all_child_lines(tBomLine,&iNumberOfchild,&tBomChildren);

					iFail=BOM_line_look_up_attribute("bl_revision",&iAttribute);
					iFail = BOM_line_ask_attribute_tag(tBomLine,iAttribute,&tTop_Asm);
					ITEM_ask_rev_id2(tTop_Asm,&rev_id);

					if(rev_id != ITK_ok)
					{
						printf("\n\n\t\t Rev_ID is %s",rev_id);
						export_dataset(tTop_Asm);

						printf("\n\n\t\t iNumberOfchild = %d",iNumberOfchild);
						if(iNumberOfchild>0)
						{
							bom_sub_child(tBomChildren,iNumberOfchild);

						}
					}
					else
					{
						printf("\n\n\t\t item is not released");
						EMH_ask_error_text(iFail,&cError);
						printf("\n\n\t\t Error : %s",cError);
					}
					BOM_close_window(tWindow);
					fclose(pFile);
				/*}
				else
				{
					EMH_ask_error_text(iFail,&cError);
					printf("\n\n Please Enter correct revision rule Error: %s",cError);
				}*/
	
			}
			else
			{
				EMH_ask_error_text(iFail,&cError);
				printf("\n\n\t\t Item not found Error : %s",cError);
			}			
		}
		else
		{
			EMH_ask_error_text(iFail,&cError);
			printf("\n\n\t\t Error : %s",cError);
		}
	}
	else
	{
		printf("\n\n\t\t Enter -item_id= ?");
		exit(0);
	}
	return iFail;
}

int bom_sub_child(tag_t * tBomChildren,int iNumberOfchild)
{
	int i=0,j=0,count=0;
	int iFail=ITK_ok;
	int iAttribute=0;
	int iNumberOfSubChild;	
	tag_t tChild=NULLTAG;
	tag_t *tSubChilds=NULLTAG;
	char * cObjectType=NULL;
	tag_t *tSecObjlist=NULLTAG;
	char cChildName[WSO_name_size_c+1];
	char cChildDes[WSO_name_size_c+1];
	char cChildType[WSO_name_size_c+1];
	char *rev_id = NULL; 
	char* cValue=NULL;

	char* cTemp = NULL;
	tag_t tTemp = NULLTAG;

	iFail=BOM_line_look_up_attribute("bl_revision",&iAttribute);

	for(j=0;j<iNumberOfchild;j++)
	{
		iFail = BOM_line_ask_attribute_tag(tBomChildren[j],iAttribute,&tChild);
		ITEM_ask_rev_id2(tChild,&rev_id);
		if(rev_id != ITK_ok)
		{
			WSOM_ask_name(tChild,cChildName);
			WSOM_ask_description(tChild,cChildDes);
			WSOM_ask_object_type(tChild,cChildType); 			
			printf("\n\n\t\t child name: %s",cChildName);
			printf("\n\n\t\t  rev  ID  : %s",rev_id);
			BOM_line_ask_all_child_lines(tBomChildren[j],&iNumberOfSubChild,&tSubChilds);
			export_dataset(tChild);
			if(iNumberOfSubChild>0)
			{	
				bom_sub_child(tSubChilds,iNumberOfSubChild);
			}
			if(tSubChilds)
			{
				MEM_free(tSubChilds);
			}
			if(tSecObjlist)
			{
				MEM_free(tSecObjlist);
			}

		}
		else
		{
			write_csv_file2(tBomChildren[j]);
		}

	}
	return iFail;
}

void export_dataset(tag_t tItem)
{
	int iCount=0,i=0,j=0,iFoundDataSet=0;
	tag_t *tSecObjlist=NULLTAG,*tRefObjectList=NULLTAG;
	tag_t iFail;
	char *cObjectType,*cError;
	char  cRefObjName[IMF_filename_size_c + 1];
	char cDestFilePath[100];
	boolean isExported = false;
	if(tItem!= NULLTAG)
	{
		iFail=GRM_list_secondary_objects_only(tItem,NULLTAG,&iCount,&tSecObjlist);
		printf("\n\n\t\t secondary objects count: %d",iCount);
		for(i=0;i<iCount;i++)
		{
			WSOM_ask_object_type2(tSecObjlist[i],&cObjectType);
			printf("\n\n\t\t cObjectType: %s",cObjectType);

			if(tc_strcmp(cObjectType,"Document")==0)
			{
				export_latest_rel_data(tSecObjlist[i]);
			}
			if(tc_strcmp(cObjectType,"H4_Drawing_Type")==0)
			{
				printf("\n\n\t\t Calling function for export_latest_rel_data H4_Drawing_Type..\n");
				export_latest_rel_data(tSecObjlist[i]);
			}
			if(tc_strcmp(cObjectType,"A2Cdocument")==0)
			{
				export_latest_rel_data(tSecObjlist[i]);
			}
			if(tc_strcmp(cObjectType,"Text")==0 || tc_strcmp(cObjectType,"PDF")==0)
			{
				AE_ask_dataset_named_refs(tSecObjlist[i],&iFoundDataSet,&tRefObjectList);
				printf("\n\n\t\t No of referance object found: %d",iFoundDataSet);
				for(j=0;j<iFoundDataSet;j++)
				{
					iFail = IMF_ask_original_file_name(tRefObjectList[j],cRefObjName);
					printf("\n\n\t\t cRefObjName: %s",cRefObjName);
					tc_strcpy(cDestFilePath," ");
					tc_strcpy(cDestFilePath, cFolderPath);
					tc_strcat(cDestFilePath,"\\");
					tc_strcat(cDestFilePath,cRefObjName);
					AOM_refresh(tSecObjlist[i],1);
					//printf("\n\n\t\t cDestFilePath: %s",cDestFilePath);
					printf("\n\n\t\t refName: %s",cObjectType);

					if(tc_strcmp(cObjectType,"PDF")==0)
						iFail=AE_export_named_ref(tSecObjlist[i],"PDF_Reference",cDestFilePath);
					else
						iFail=AE_export_named_ref(tSecObjlist[i],cObjectType,cDestFilePath);		
					if(iFail==ITK_ok)
					{
						isExported =true;
						printf("\n\n\t\t Data Exported succesffuly!!");

					}
					else
					{
						EMH_ask_error_text(iFail,&cError);
						printf("\n\n\t\t Error ::%s!!",cError);
					}
				}
			}
		}
		if(tSecObjlist)
		{
			MEM_free(tSecObjlist);
		}
		if(tRefObjectList)
		{
			MEM_free(tRefObjectList);
		}
		write_csv_file(tItem,isExported);
	}
	else
	{
		printf("\n\n\t\t not able to find Item tag");
	}

}

void export_latest_rel_data(tag_t tItemtag)
{
	int iCount=0,i=0;
	tag_t *tItemRevList = NULLTAG;
	char* cPropValue = NULL;
	char *cItemName = NULL;
	ITEM_list_all_revs(tItemtag,&iCount,&tItemRevList);
	printf("\n\n\t\t No of HON Drawing Type Revision found: %d",iCount);
	for (i = iCount-1; i >= 0; --i)
	{
		printf("\n\n\t\t finding released status..");

		CHECK_ERROR(AOM_UIF_ask_value(tItemRevList[i],"release_status_list",&cPropValue));
		printf("\n\n\t\t found released status..");

		if(tc_strcmp(cPropValue,"Released")==0)
		{
			printf("\n\n\t\t No of HON Drawing Type status is released..");
			printf("\n\n\t\t getting Item Rev ID");
			CHECK_ERROR(ITEM_ask_rev_id2(tItemRevList[i],&cItemName));
			printf("\n\n\t\t %s Item is Released...",cItemName);
			export_dataset(tItemRevList[i]);
			break;
		}
	}
}

int create_export_folder(char* cFolderPath)
{
	int check;
	//printf("\n\n\t\t Folder Path :: %s",cFolderPath);
	check = mkdir(cFolderPath);
	//check = mkdir("E:\Prognuer\000081_A");
	//printf("\n\n\t\t Check Val %d",check);
	if(!check)
		printf("\n\n\t\t Directory created sucessfully!!");
	else
		printf("\n\n\t\t Directory already exist!!");
	return check;
}

char* get_folder_name(char* cItem_id) 
{
	char* cTemp = NULL;
	char* item_id_rev = NULL;
	char* cFolderName = NULL;
	
	char *timestamp = (char *)MEM_alloc(sizeof(char) * 16);
	time_t ltime;
	struct tm *tm;
	ltime=time(NULL);
	tm=localtime(&ltime);
	sprintf(timestamp,"%02d_%02d_%04d_%02d_%02d_%02d", tm->tm_mday,tm->tm_mon+1, 
		tm->tm_year+1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

	cTemp = tc_strcat("\\",cItem_id);
	item_id_rev = tc_strcat(cTemp,"_");
	cFolderName = tc_strcat(item_id_rev,timestamp);
	//printf("\n\n\t\t cFolderName :: %s",cFolderName);
	return cFolderName;
}
void write_csv_file(tag_t tRevTag,boolean isExported)
{
	tag_t tItem;
	char* cItem_Id =NULL;
	char* cItem_Rev =NULL;
	char* cItem_Name =NULL;
	char* cStatus = NULL;
	char* cAction = NULL;
	char* cLatestRev = NULL;
	tag_t cLatestRevTag = NULLTAG;
	char* cPropValue=NULL;
	char* cLatestRevAvailable = false;
	printf("\n\n\t\t File csv file wrting started for...");

	ITEM_ask_item_of_rev(tRevTag,&tItem);
	CHECK_ERROR(ITEM_ask_id2(tItem,&cItem_Id));
	//printf("\n cItem_Id %s...",cItem_Id);
	CHECK_ERROR(ITEM_ask_rev_id2(tRevTag,&cItem_Rev));
	//printf("\n cItem_Rev %s...",cItem_Rev);
	CHECK_ERROR(ITEM_ask_name2(tItem,&cItem_Name));
	//printf("\n cItem_Name %s...",cItem_Name);
	CHECK_ERROR(AOM_UIF_ask_value(tRevTag,"release_status_list",&cPropValue));
	//printf("\n cPropValue %s...",cPropValue);
	CHECK_ERROR(ITEM_ask_latest_rev(tItem,&cLatestRevTag));

	CHECK_ERROR(ITEM_ask_rev_id2(cLatestRevTag,&cLatestRev));
	//printf("\n ccLatestRev %s...",cLatestRev);

	if(isExported)
		cAction = "Exported";
	else
		cAction = "Not Exported";
	//printf("\n isExported %d",isExported);
	if (tc_strcmp(cItem_Rev,cLatestRev)==0)
	{
		fprintf(pFile,"\n%s,%s,%s,%s,%s,%s,%s,%s,%s",
			cItem_Id,cItem_Rev,cItem_Name,cPropValue,cAction,"NA","NA","NA","NA");
	}
	else
	{
		fprintf(pFile,"\n%s,%s,%s,%s,%s,%s,%s,%s,%s",
			cItem_Id,cItem_Rev,cItem_Name,cPropValue,cAction,"Y",cLatestRev,"Working","Skipped");
	}

}

void write_csv_file2(tag_t tBomLine)
{
	tag_t tItem = NULLTAG;
	char* cItem_Id =NULL;
	char* cItem_Rev =NULL;
	char* cItem_Name =NULL;
	char* cStatus = NULL;
	char* cAction = NULL;
	char* cLatestRev = NULL;
	tag_t cLatestRevTag = NULLTAG;
	char* cPropValue=NULL;
	char* cLatestRevAvailable = false;
	int iAttribute=0;

	printf("\n\n\t\t File csv file2 wrting started for...");

	CHECK_ERROR(BOM_line_look_up_attribute("bl_item_item_id",&iAttribute));
	CHECK_ERROR(BOM_line_ask_attribute_string (tBomLine,iAttribute,&cItem_Id));

	CHECK_ERROR(BOM_line_look_up_attribute("bl_item_object_name",&iAttribute));
	CHECK_ERROR(BOM_line_ask_attribute_string (tBomLine,iAttribute,&cItem_Name));

	CHECK_ERROR(BOM_line_look_up_attribute("bl_rev_item_revision_id",&iAttribute));
	CHECK_ERROR(BOM_line_ask_attribute_string (tBomLine,iAttribute,&cItem_Rev));

	fprintf(pFile2,"\n%s,%s,%s,%s,%s",
		cItem_Id,cItem_Rev,cItem_Name,"Working","Failed Rev Rule");

}

