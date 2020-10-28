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
#include <base_utils\IFail.hxx> 
#include <base_utils\TcResultStatus.hxx>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Teamcenter;
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


int bom_sub_child(tag_t* tBomChildren,int iNumberOfchild);
void export_dataset(tag_t tItem,string);
void export_latest_rel_data(tag_t tDocument);
int create_export_folder(string);
void write_csv_file(tag_t,boolean);
void write_csv_file2(tag_t);

ofstream pFile;
ofstream pFile2;
string  cFolderPath;

int ITK_user_main(int argc,char *argv[]) 
{
	ResultStatus rStatus = ITK_ok;
	int iFail=ITK_ok;
	int i=0,j=0,k=0;
	int iRevCount=0;
	int iNumberOfchild=0;
	tag_t tItem=NULLTAG;
	scoped_smptr<tag_t> tRevList;
	tag_t tWindow=NULLTAG;
	tag_t tBomLine=NULLTAG;
	scoped_smptr<tag_t> tBomChildren;
	tag_t tRule=NULLTAG;
	int iAttribute =0;
	tag_t tTop_Asm = NULLTAG;
	scoped_smptr<char> rev_id;
	char cExportLog[100];   // Don't user char array
	char cNotExportLog[100]; // Don't use char array
	int iCheckFolderCreation = 0;
	scoped_smptr<char> cAttribute_value;
	int inOfItem = 0;
	scoped_smptr<tag_t> tItemTagList;

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
			cout<<"\n\n\t\t Please enter correct arguments.."<<endl;
			cout<<"\n\n\t\t e.g mass_download_datasets_bvr.exe -u=infodba -p=infodba -g=dba -item_id=001 -rev_rule=\"Any Status; No Working\" -folder_path=\"E:\Files\""<<endl;
			return ITK_ok;
		}

		if(!cItem_id.empty())
		{
			rStatus=ITK_init_module(u.c_str(),p.c_str(),g.c_str());			
			cout<<"\n\n\t\t Login Successful"<<endl;
			cout<<"\n\n\t\t Trying to find Item "<<cItem_id.c_str()<<endl;
			int iNumOfItem = 0;
			scoped_smptr<tag_t> tItemTagList;
			string sAttValue;
			scoped_smptr<char> objType;
			sAttValue = "item_id="+cItem_id;
			ITEM_find_items_by_string(sAttValue.c_str(),&iNumOfItem,&tItemTagList);
			for (int i = 0; i < iNumOfItem; i++)
			{
				WSOM_ask_object_type2(tItemTagList[i],&objType);
				cout<<"\n\n\t\t Object Type is ::" << objType.get() <<endl;
				if(tc_strcmp(objType.get(),"H4_Hon_Part_Type")==0)
				//if(tc_strcmp(objType.get(),"A2H4_Hon_Part")==0)
				{
					tItem = tItemTagList[i];
					cout<<"\n\n\t\t H4_Hon_Part_Type Object Type found"<<endl;
					break;
				}
			}
			if(tItem!=NULLTAG)
			{
				iCheckFolderCreation = create_export_folder(cFolderPath.c_str());
				tc_strcpy(cExportLog," ");
				tc_strcpy(cExportLog, cFolderPath.c_str());
				tc_strcat(cExportLog,"\\dataset_exported_log.csv");

				tc_strcpy(cNotExportLog," ");
				tc_strcpy(cNotExportLog, cFolderPath.c_str());
				tc_strcat(cNotExportLog,"\\dataset_not_exported_log.csv");

				pFile.open(cExportLog);
				pFile2.open(cNotExportLog);
				pFile<<"Item ID","Rev","Name","Status","Action","Lastest Rev Avaialble","Rev","Status","Action\n";

				pFile<<"Item ID","Rev","Name","Status","Reason\n";

				rStatus=BOM_create_window(&tWindow);  //Use Rstatus instead of Ifail for all ITK calls
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
				rStatus = BOM_save_window(tWindow);

				rStatus=BOM_set_window_top_line(tWindow,tItem,NULLTAG,NULLTAG,&tBomLine);  //Use Rstatus instead of Ifail for all ITK calls
				rStatus=BOM_line_ask_all_child_lines(tBomLine,&iNumberOfchild,&tBomChildren);  //Use Rstatus instead of Ifail for all ITK calls

				rStatus=BOM_line_look_up_attribute("bl_revision",&iAttribute);  //Use Rstatus instead of Ifail for all ITK calls
				rStatus = BOM_line_ask_attribute_tag(tBomLine,iAttribute,&tTop_Asm);  //Use Rstatus instead of Ifail for all ITK calls
				rStatus = ITEM_ask_rev_id2(tTop_Asm,&rev_id);   //Use Rstatus instead of Ifail for all ITK calls

				if(rev_id != ITK_ok)
				{
					cout<<"\n\n\t\t Rev_ID is "<<rev_id.get()<<endl;
					export_dataset(tTop_Asm,"H4_Source_File");
					export_dataset(tTop_Asm,"IMAN_reference");
					cout<<"\n\n\t\t iNumberOfchild = "<<iNumberOfchild<<endl;
					if(iNumberOfchild>0)
					{
						rStatus=bom_sub_child(tBomChildren.get(),iNumberOfchild);

					}
				}
				else
				{
					cout<<"\n\n\t\t item is not released"<<endl;
				}
				rStatus = BOM_close_window(tWindow);  //Use Rstatus instead of Ifail for all ITK calls
				pFile2.close();
				pFile.close();
			}
			else
			{
				cout<<"\n\n\t\t Item not found Error : "<<endl;
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
	return ITK_ok;
}

int bom_sub_child(tag_t* tBomChildren,int iNumberOfchild)
{
	ResultStatus rStatus;
	int i=0,j=0,count=0;
	int iFail=ITK_ok;
	int iAttribute=0;
	int iNumberOfSubChild;	
	tag_t tChild=NULLTAG;
	scoped_smptr<tag_t> tSubChilds;
	scoped_smptr<tag_t> tSecObjlist;
	char cChildName[WSO_name_size_c+1];  // Don't user char array
	char cChildDes[WSO_name_size_c+1];    // Same
	char cChildType[WSO_name_size_c+1];   // Same
	scoped_smptr<char> rev_id;

	try{
		rStatus=BOM_line_look_up_attribute("bl_revision",&iAttribute);   //Use Rstatus instead of Ifail for all ITK calls

		for(j=0;j<iNumberOfchild;j++)
		{
			rStatus = BOM_line_ask_attribute_tag(tBomChildren[j],iAttribute,&tChild); //Use Rstatus instead of Ifail for all ITK calls
			rStatus = ITEM_ask_rev_id2(tChild,&rev_id); //Use Rstatus instead of Ifail for all ITK calls
			if(rev_id != ITK_ok)
			{
				rStatus = WSOM_ask_name(tChild,cChildName); //Use Rstatus instead of Ifail for all ITK calls
				rStatus = WSOM_ask_description(tChild,cChildDes); //Use Rstatus instead of Ifail for all ITK calls
				rStatus = WSOM_ask_object_type(tChild,cChildType); 		 //Use Rstatus instead of Ifail for all ITK calls	
				cout<<"\n\n\t\t child name:"<<cChildName<<endl;
				cout<<"\n\n\t\t  rev  ID  : "<<rev_id.get()<<endl;
				rStatus = BOM_line_ask_all_child_lines(tBomChildren[j],&iNumberOfSubChild,&tSubChilds);  
				export_dataset(tChild,"H4_Source_File");
				export_dataset(tChild,"IMAN_reference");
				if(iNumberOfSubChild>0)
				{	
					rStatus=bom_sub_child(tSubChilds.get(),iNumberOfSubChild);  //Use Rstatus instead of Ifail for all ITK calls
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


	return ITK_ok;
}

void export_dataset(tag_t tItem,string cRelation)
{
	int iCount=0,i=0,j=0,k=0,iFoundDataSet=0,ref_count=0;
	scoped_smptr<tag_t> tSecObjlist;
	scoped_smptr<tag_t> tRefObjectList;
	ResultStatus rStatus;
	tag_t iFail;
	tag_t tDatasetType = NULLTAG;
	tag_t tRelationTag=NULLTAG;
	scoped_smptr<char*>ref_list;
	scoped_smptr<char> cObjectType; 
	scoped_smptr<char> cError;
	char  cRefObjName[IMF_filename_size_c + 1];
	char cDestFilePath[100];  
	boolean isExported = false;

	try{
		if(tItem!= NULLTAG)
		{

			if(tc_strcmp(cRelation.c_str(),"H4_Source_File")==0)
				iFail=GRM_find_relation_type("H4_Source_File",&tRelationTag);
			if(tc_strcmp(cRelation.c_str(),"IMAN_reference")==0)
				iFail=GRM_find_relation_type("IMAN_reference",&tRelationTag);

			rStatus=GRM_list_secondary_objects_only(tItem,tRelationTag,&iCount,&tSecObjlist);
			cout<<"\n\n\t\t secondary objects count:"<< iCount <<endl;
			for(i=0;i<iCount;i++)
			{
				rStatus=WSOM_ask_object_type2(tSecObjlist[i],&cObjectType);

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
					rStatus = IMF_ask_original_file_name(tRefObjectList[j],cRefObjName);
					cout<<"\n\n\t\t cRefObjName: "<<cRefObjName<<endl;
					tc_strcpy(cDestFilePath," ");
					tc_strcpy(cDestFilePath, cFolderPath.c_str());
					tc_strcat(cDestFilePath,"\\");
					tc_strcat(cDestFilePath,cRefObjName);
					AOM_refresh(tSecObjlist[i],1);
					cout<<"\n\n\t\t refName: "<<cObjectType.get()<<endl;
					cout<<"\n\n\t\t cDestFilePath::"<<cDestFilePath<<endl;
					rStatus=AE_ask_dataset_datasettype(tSecObjlist[i],&tDatasetType);
					iFail = AE_ask_datasettype_refs (tDatasetType, &ref_count, &ref_list);
					cout<<"\n\n\t\t No Dataset ref object is %d"<<ref_count<<endl;
					if( iFail == ITK_ok )
					{
						for (k = 0; k < ref_count; k++)
						{
							cout<<"\n\n\t\t named reference  is /n"<< k + 1<< ref_list[k]<<endl;
							iFail=AE_export_named_ref(tSecObjlist[i],ref_list[k],cDestFilePath);
						}
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
	scoped_smptr<tag_t> tItemRevList;
	scoped_smptr<char> cPropValue;
	scoped_smptr<char> cItemName;
	try
	{
		rStatus = ITEM_list_all_revs(tItemtag,&iCount,&tItemRevList);
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
		check = _mkdir(cFolder_Path.c_str());
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
	scoped_smptr<char>  cItem_Id;
	scoped_smptr<char> cItem_Rev;
	scoped_smptr<char> cItem_Name;
	scoped_smptr<char> cAction;
	scoped_smptr<char> cLatestRev;
	scoped_smptr<char> cPropValue;

	cout<<"\n\n\t\t File csv file wrting started for..."<<endl;

	try
	{
		cout<<"writing started in csv file"<<endl;
		
		rStatus = ITEM_ask_item_of_rev(tRevTag,&tItem);

		rStatus = ITEM_ask_id2(tItem,&cItem_Id);

		rStatus = ITEM_ask_rev_id2(tRevTag,&cItem_Rev);

		rStatus=ITEM_ask_name2(tItem,&cItem_Name);

		rStatus = AOM_UIF_ask_value(tRevTag,"release_status_list",&cPropValue);

		rStatus = ITEM_ask_latest_rev(tItem,&cLatestRevTag);

		rStatus = ITEM_ask_rev_id2(cLatestRevTag,&cLatestRev);
		cout<<"Writing finshed"<<endl;
		if(isExported)
		{
			cAction = "Exported";
			cout<<"Writing finshed"<<endl;
		}
		else
		{
			cAction = "Not Exported";
			cout<<"Writing finshed"<<endl;
		}
		//if (tc_strcmp(cItem_Rev,cLatestRev.get())==0)
		{

			/*fprintf(pFile.get(),"\n%s,%s,%s,%s,%s,%s,%s,%s,%s",
			cItem_Id,cItem_Rev,cItem_Name,cPropValue,cAction,"Y",cLatestRev,"Working","Skipped");*/
			cout<<"Writing finshed112"<<endl;
			pFile << cItem_Id.get(),cItem_Rev.get(),cItem_Name.get(),cPropValue.get(),cAction.get(),"Y",cLatestRev.get(),"Working","Skipped";
			cout<<"Writing finshed11"<<endl;
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
	scoped_smptr<char> cItem_Id;
	scoped_smptr<char> cItem_Rev;
	scoped_smptr<char> cItem_Name;

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

		pFile2 << cItem_Id.get(),cItem_Rev.get(),cItem_Name.get(),"Working","Failed Rev Rule";

	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}

}


