#include<iostream>
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
#include<string>
#include<base_utils\ScopedSmPtr.hxx>   
#include <base_utils/IFail.hxx>                 // Either have "\" or "/"
#include <base_utils/TcResultStatus.hxx>

using namespace std;
// Add using namespace teamcenter here. Avoids writing teamcenter:: everytime.

// The file will be maintained by developers who didn't work on creating this utility. It is our job to make their lives easy by adding correct comments.
// If the comments are missing they will trouble us later. Its better we make this file readable and understandable for new developers now rather than spending time on it later.
// Please add comments to each function and at important If conditions.
// Please write down the logic used in simple words where ever needed.

// Main category of comments
// 1) Use Smart pointers where ever memory is being allocated by teamcenter ITK's. Don't use mem_free. Not sure when and how the code will crash while freeing the memory.
      // Allocating and freeing memory should not be applications responsibility. Please take it as a rule of thumb.
// 2) Use Resultstatus for all Teamcenter ITK calls.
// 3) Avoid using char[] arrays. Its a very bad habbit and can lead to crashes and unresolvable bus in future.
// 4) File handling using c++ constructs is easy, simple and clean. Not very clear why we are using fprintf. My recommendation would be to use c++ constructs.
// 5) Add clear and simple comments to the code exlaing what exactly the code is doing.


int bom_sub_child(tag_t * tBomChildren,int iNumberOfchild);
void export_dataset(tag_t tItem,char*);
void export_latest_rel_data(tag_t tDocument);
int create_export_folder(string);
void write_csv_file(tag_t,boolean);
void write_csv_file2(tag_t);

FILE *pFile=NULL; // This should be smart pointer.
FILE *pFile2=NULL; // This should be smart pointer.
string  cFolderPath;

int ITK_user_main(int argc,char *argv[]) 
{
	ResultStatus rStatus = ITK_ok;
	Teamcenter::scoped_smptr<char> cError;
	int iFail=ITK_ok;
	int i=0,j=0,k=0;
	int iRevCount=0;
	int iNumberOfchild=0;
	tag_t tItem=NULLTAG;
	tag_t *tRevList=NULLTAG;	
	tag_t tWindow=NULLTAG;
	tag_t tBomLine=NULLTAG;
	tag_t *tBomChildren=NULLTAG;
	tag_t tRule=NULLTAG;
	int iAttribute =0;
	tag_t tTop_Asm = NULLTAG;
	Teamcenter::scoped_smptr<char> rev_id;
	char cExportLog[100];   // Don't user char array
	char cNotExportLog[100]; // Don't use char array
	int iCheckFolderCreation = 0;
	Teamcenter::scoped_smptr<char> cAttribute_value;
	int inOfItem = 0;
	tag_t* tItemTagList = NULLTAG;  // Smart pointer
	try
	{
		string u= ITK_ask_cli_argument ("-u=");
		string p= ITK_ask_cli_argument ("-p=");
		string g= ITK_ask_cli_argument ("-g=");
		string cItem_id=ITK_ask_cli_argument("-item_id=");
		string cRevRule=ITK_ask_cli_argument("-rev_rule=");
		string cFilePath=ITK_ask_cli_argument("-folder_path=");
		cFolderPath=cFilePath;

		if(argc==7)
		{

		}
		else
		{
			cout<<"\n\n\t\t Please enter correct arguments.. "<<endl;
			cout<<"\n\n\t\t e.g mass_download_datasets_bvr.exe -u=infodba -p=infodba -g=dba -item_id=001 -rev_rule=Any Status; No Working -folder_path=E:\Files"<<endl;
			return ITK_ok;
		}

		if(!cItem_id.empty())
		{
			iFail=ITK_init_module(u.c_str(),p.c_str(),g.c_str());
			if (iFail==ITK_ok)
			{
				cout<<"\n\n\t\t Login Successful"<<endl;
				cout<<"\n\n\t\t Trying to find Item "<<cItem_id.c_str()<<endl;
				iFail=ITEM_find_item_in_idcontext(cItem_id.c_str(),NULLTAG,&tItem);  //Use Rstatus instead of Ifail for all ITK calls
				if((iFail==ITK_ok)&&(tItem!=NULLTAG))

				{
					iCheckFolderCreation = create_export_folder(cFolderPath.c_str());
					tc_strcpy(cExportLog," ");
					tc_strcpy(cExportLog, cFolderPath.c_str());
					tc_strcat(cExportLog,"\\dataset_exported_log.csv");

					tc_strcpy(cNotExportLog," ");
					tc_strcpy(cNotExportLog, cFolderPath.c_str());
					tc_strcat(cNotExportLog,"\\dataset_not_exported_log.csv");

					pFile=fopen(cExportLog,"w+");
					pFile2=fopen(cNotExportLog,"w+");

					fprintf(pFile,"\n%s,%s,%s,%s,%s,%s,%s,%s,%s",
						"Item ID","Rev","Name","Status","Action","Lastest Rev Avaialble","Rev","Status","Action");

					fprintf(pFile2,"\n%s,%s,%s,%s,%s",
						"Item ID","Rev","Name","Status","Reason");
					iFail=BOM_create_window(&tWindow);  //Use Rstatus instead of Ifail for all ITK calls
					cout<<"\n\n\t\t Finding Revision Rule  "<<cRevRule.c_str()<<endl;
					if(tc_strcmp(cRevRule.c_str(),"Any Status; No Working")==0 || tc_strcmp(cRevRule.c_str(),"Any Status; Working")==0 ||tc_strcmp(cRevRule.c_str(),"Latest Working")==0)
					{
						if(tc_strcmp(cRevRule.c_str(),"Any Status; No Working")==0)
							CFM_find("Any Status; No Working",&tRule);
						if(tc_strcmp(cRevRule.c_str(),"Any Status; Working")==0)
							CFM_find("Any Status; Working",&tRule);
						if(tc_strcmp(cRevRule.c_str(),"Latest Working")==0)
							CFM_find("Latest Working",&tRule);
					}
					else
					{

						cout<<"\n\n Please Enter correct revision rule Error"<<endl;
						return ITK_ok;
					}

					rStatus= BOM_set_window_config_rule( tWindow, tRule ); 
					cout<<"\n\n\t\t Revision Rule Set "<<endl;
					BOM_save_window(tWindow);

					iFail=BOM_set_window_top_line(tWindow,tItem,NULLTAG,NULLTAG,&tBomLine);  //Use Rstatus instead of Ifail for all ITK calls
					iFail=BOM_line_ask_all_child_lines(tBomLine,&iNumberOfchild,&tBomChildren);  //Use Rstatus instead of Ifail for all ITK calls

					iFail=BOM_line_look_up_attribute("bl_revision",&iAttribute);  //Use Rstatus instead of Ifail for all ITK calls
					iFail = BOM_line_ask_attribute_tag(tBomLine,iAttribute,&tTop_Asm);  //Use Rstatus instead of Ifail for all ITK calls
					ITEM_ask_rev_id2(tTop_Asm,&rev_id);   //Use Rstatus instead of Ifail for all ITK calls
  
					if(rev_id != ITK_ok)
					{
						cout<<"\n\n\t\t Rev_ID is "<<rev_id.get()<<endl;
						export_dataset(tTop_Asm,"IMAN_reference");

						cout<<"\n\n\t\t iNumberOfchild = "<<iNumberOfchild<<endl;
						if(iNumberOfchild>0)
						{
							bom_sub_child(tBomChildren,iNumberOfchild);

						}
					}
					else
					{
						cout<<"\n\n\t\t item is not released"<<endl;
						EMH_ask_error_text(iFail,&cError);
						cout<<"\n\n\t\t Error : "<<cError.get()<<endl;
					}
					BOM_close_window(tWindow);  //Use Rstatus instead of Ifail for all ITK calls
					fclose(pFile2);
					fclose(pFile);

				}
				else
				{
					EMH_ask_error_text(iFail,&cError);  //Use Rstatus instead of Ifail for all ITK calls
					cout<<"\n\n\t\t Item not found Error : "<<cError.get()<<endl;
				}			
			}
			else
			{
				EMH_ask_error_text(iFail,&cError);  //Use Rstatus instead of Ifail for all ITK calls
				cout<<"\n\n\t\t Error : "<<cError.get()<<endl;
			}
		}
		else
		{
			cout<<"\n\n\t\t Enter -item_id= ?"<<endl;
			exit(0);
		}
	}
	catch(IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
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
	tag_t *tSecObjlist=NULLTAG;
	char cChildName[WSO_name_size_c+1];  // Don't user char array
	char cChildDes[WSO_name_size_c+1];    // Same
	char cChildType[WSO_name_size_c+1];   // Same
	Teamcenter::scoped_smptr<char> rev_id;


	try{
		iFail=BOM_line_look_up_attribute("bl_revision",&iAttribute);   //Use Rstatus instead of Ifail for all ITK calls

		for(j=0;j<iNumberOfchild;j++)
		{
			iFail = BOM_line_ask_attribute_tag(tBomChildren[j],iAttribute,&tChild); //Use Rstatus instead of Ifail for all ITK calls
			ITEM_ask_rev_id2(tChild,&rev_id); //Use Rstatus instead of Ifail for all ITK calls
			if(rev_id != ITK_ok)
			{
				WSOM_ask_name(tChild,cChildName); //Use Rstatus instead of Ifail for all ITK calls
				WSOM_ask_description(tChild,cChildDes); //Use Rstatus instead of Ifail for all ITK calls
				WSOM_ask_object_type(tChild,cChildType); 		 //Use Rstatus instead of Ifail for all ITK calls	
				cout<<"\n\n\t\t child name:"<<cChildName<<endl;
				cout<<"\n\n\t\t  rev  ID  : "<<rev_id.get()<<endl;
				BOM_line_ask_all_child_lines(tBomChildren[j],&iNumberOfSubChild,&tSubChilds);  //Use Rstatus instead of Ifail for all ITK calls
				export_dataset(tChild,"IMAN_reference");
				if(iNumberOfSubChild>0)
				{	
					bom_sub_child(tSubChilds,iNumberOfSubChild);  //Use Rstatus instead of Ifail for all ITK calls
				}
				if(tSubChilds)
				{
					MEM_free(tSubChilds); // Use smart pointers. You don't have to free any memory
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
	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}


	return iFail;
}

void export_dataset(tag_t tItem,char *cRelation)
{
	int iCount=0,i=0,j=0,k=0,iFoundDataSet=0,ref_count=0;
	tag_t *tSecObjlist=NULLTAG,*tRefObjectList=NULLTAG;    // Smart pointer should be used
	tag_t iFail;
	tag_t tDatasetType = NULLTAG;
	tag_t tRelationTag=NULLTAG;
	Teamcenter::scoped_smptr<char*>ref_list;
	Teamcenter::scoped_smptr<char> cObjectType; 
	Teamcenter::scoped_smptr<char> cError;
	char  cRefObjName[IMF_filename_size_c + 1];
	char cDestFilePath[100];  //No char array
	boolean isExported = false;

	try{
		if(tItem!= NULLTAG)
		{

			if(tc_strcmp(cRelation,"H4_Source_File")==0)
				iFail=GRM_find_relation_type("H4_Source_File",&tRelationTag);
			if(tc_strcmp(cRelation,"IMAN_reference")==0)
				iFail=GRM_find_relation_type("IMAN_reference",&tRelationTag);

			iFail=GRM_list_secondary_objects_only(tItem,tRelationTag,&iCount,&tSecObjlist);
			printf("\n\n\t\t secondary objects count: %d",iCount);
			for(i=0;i<iCount;i++)
			{
				WSOM_ask_object_type2(tSecObjlist[i],&cObjectType);

				cout<<"\n\n\t\t cObjectType: "<<cObjectType.get()<<endl;

				if(tc_strcmp(cObjectType.get(),"MISC")==0)
				{
					continue;
				}
				if(tc_strcmp(cObjectType.get(),"Document")==0)
				{
					export_latest_rel_data(tSecObjlist[i]);
				}
				if(tc_strcmp(cObjectType.get(),"H4_Drawing_Type")==0)
				{
					cout<<"\n\n\t\t Calling function for export_latest_rel_data H4_Drawing_Type..\n"<<endl;
					export_latest_rel_data(tSecObjlist[i]);
				}
				if(tc_strcmp(cObjectType.get(),"A2Cdocument")==0)
				{
					export_latest_rel_data(tSecObjlist[i]);
				}


				AE_ask_dataset_named_refs(tSecObjlist[i],&iFoundDataSet,&tRefObjectList);
				cout<<"\n\n\t\t No of referance object found: "<<iFoundDataSet<<endl;
				for(j=0;j<iFoundDataSet;j++)
				{
					iFail = IMF_ask_original_file_name(tRefObjectList[j],cRefObjName);
					cout<<"\n\n\t\t cRefObjName: "<<cRefObjName<<endl;
					tc_strcpy(cDestFilePath," ");
					tc_strcpy(cDestFilePath, cFolderPath.c_str());
					tc_strcat(cDestFilePath,"\\");
					tc_strcat(cDestFilePath,cRefObjName);
					AOM_refresh(tSecObjlist[i],1);
					cout<<"\n\n\t\t refName: "<<cObjectType.get()<<endl;
					cout<<"\n\n\t\t cDestFilePath::"<<cDestFilePath<<endl;
					AE_ask_dataset_datasettype(tSecObjlist[i],&tDatasetType);
					iFail = AE_ask_datasettype_refs (tDatasetType, &ref_count, &ref_list);
					cout<<"\n\n\t\t No Dataset ref object is %d"<<ref_count<<endl;
					if( iFail == ITK_ok )
					{
						for (k = 0; k < ref_count; k++)
						{
							cout<<"\n\n\t\t named reference  is /n"<< k + 1<< ref_list[k]<<endl;
							iFail=AE_export_named_ref(tSecObjlist[i],ref_list[k],cDestFilePath);
						}
						//MEM_free (ref_list.get());
					}
					else
					{
						cout<<"\n\n\t\t Error  asking for datasettype references./n"<< iFail<<endl;
					}

					if(iFail==ITK_ok)
					{
						isExported =true;
						cout<<"\n\n\t\t Data Exported succesffuly!!"<<endl;

					}
					else
					{
						EMH_ask_error_text(iFail,&cError);
						cout<<"\n\n\t\t Error ::!!"<<cError.get()<<endl;
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
			//write_csv_file(tItem,isExported);
		}
		else
		{
			cout<<"\n\n\t\t not able to find Item tag"<<endl;
		}

	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}


}

void export_latest_rel_data(tag_t tItemtag)
{
	ResultStatus rStatus;
	int iCount=0,i=0;
	tag_t *tItemRevList = NULLTAG;
	Teamcenter::scoped_smptr<char> cPropValue;
	Teamcenter::scoped_smptr<char> cItemName;
	try
	{
		ITEM_list_all_revs(tItemtag,&iCount,&tItemRevList);
		cout<<"\n\n\t\t No of HON Drawing Type Revision found: "<<iCount<<endl;
		for (i = iCount-1; i >= 0; --i)
		{
			cout<<"\n\n\t\t finding released status.."<<endl;

			rStatus=AOM_UIF_ask_value(tItemRevList[i],"release_status_list",&cPropValue);


			if(tc_strcmp(cPropValue.get(),"Released")==0)
			{
				cout<<"\n\n\t\t No of HON Drawing Type status is released.."<<endl;
				cout<<"\n\n\t\t getting Item Rev ID"<<endl;
				rStatus=ITEM_ask_rev_id2(tItemRevList[i],&cItemName);
				cout<<"\n\n\t\t  Item is Released..."<<cItemName.get()<<endl;
				export_dataset(tItemRevList[i],"H4_Source_File");

				break;
			}
		}
	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}


}


int create_export_folder(string cFolder_Path)
{
	int check;
	try{
		check = mkdir(cFolder_Path.c_str());
		if(!check)
			cout<<"\n\n\t\t Directory created sucessfully!!"<<endl;
		else
			cout<<"\n\n\t\t Directory already exist!!"<<endl;
	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}
	return check;
}

void write_csv_file(tag_t tRevTag,boolean isExported)
{
	ResultStatus rStatus;
	tag_t tItem;
	tag_t cLatestRevTag = NULLTAG;
	Teamcenter::scoped_smptr<char>  cItem_Id;
	Teamcenter::scoped_smptr<char> cItem_Rev;
	Teamcenter::scoped_smptr<char> cItem_Name;
	Teamcenter::scoped_smptr<char> cAction;
	Teamcenter::scoped_smptr<char> cLatestRev;
	Teamcenter::scoped_smptr<char> cPropValue;

	cout<<"\n\n\t\t File csv file wrting started for..."<<endl;

	try
	{
		ITEM_ask_item_of_rev(tRevTag,&tItem);
		rStatus = ITEM_ask_id2(tItem,&cItem_Id);

		rStatus = ITEM_ask_rev_id2(tRevTag,&cItem_Rev);

		rStatus=ITEM_ask_name2(tItem,&cItem_Name);

		rStatus = AOM_UIF_ask_value(tRevTag,"release_status_list",&cPropValue);

		rStatus = ITEM_ask_latest_rev(tItem,&cLatestRevTag);

		rStatus = ITEM_ask_rev_id2(cLatestRevTag,&cLatestRev);

		if(isExported)
			cAction = "Exported";
		else
			cAction = "Not Exported";

		if (tc_strcmp(cItem_Rev.get(),cLatestRev.get())==0)
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
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}

}

void write_csv_file2(tag_t tBomLine)
{

	ResultStatus rStatus;
	Teamcenter::scoped_smptr<char> cItem_Id;
	Teamcenter::scoped_smptr<char> cItem_Rev;
	Teamcenter::scoped_smptr<char> cItem_Name;

	int iAttribute=0;

	cout<<"\n\n\t\t File csv file2 wrting started for..."<<endl;

	try
	{
		rStatus= BOM_line_look_up_attribute("bl_item_item_id",&iAttribute);
		rStatus = BOM_line_ask_attribute_string (tBomLine,iAttribute,&cItem_Id);

		rStatus = BOM_line_look_up_attribute("bl_item_object_name",&iAttribute);
		rStatus = BOM_line_ask_attribute_string (tBomLine,iAttribute,&cItem_Name);

		rStatus = BOM_line_look_up_attribute("bl_rev_item_revision_id",&iAttribute);
		rStatus = BOM_line_ask_attribute_string (tBomLine,iAttribute,&cItem_Rev);

		fprintf(pFile2,"\n%s,%s,%s,%s,%s",
			cItem_Id,cItem_Rev,cItem_Name,"Working","Failed Rev Rule");
	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}

}


