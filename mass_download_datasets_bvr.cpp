/******************************************************************************************************************************
  @Requirment ::
 * Export all dataset Named reference file from  latest released revision , Document attached different relation type 
   i.e. Iman references and H4_Source_File .
 * There is two item type is used in the one Is H4_Hon_Part_Type and H4_Hon_Drowing _type
 * H4_Hon_Part_Type  is  Item type and H4_Hon_Drowing _type is Document type.
 * H4_Hon_Drowing _type is attached to H4_Hon_Part_Type  by  IMAN_referenace relation .
 * H4_Hon_Part_Type  and H4_Hon_Drowing _type  Revision has  attached different types of datasets , 
   Export data only from the latest released revision Named Reference file,  
   whose attached by relation IMAN reference and H4_Source_File  and except .mi file (ex. Miscellaneous file type should not export).

 * Use Case  : 
	There was  Custom Item type and  Document type that is H4_Hon_Part_Type and  H4_Hon_Drowing _type respectively 

 *******************************************************************************************************************************/

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

int bom_sub_child(tag_t* tBomChildren,int iNumberOfchild);
void export_dataset(tag_t tItem,string);
void export_latest_rel_data(tag_t tDocument);
int create_export_folder(string);
void write_csv_file(tag_t,boolean);
void write_csv_file2(tag_t);

fstream pFile;
fstream pFile2;
string  cFolderPath;

/**
    * This function will 
	* 1. Get the command line argument
	  2. Log in to Teamcenter
	  3. Check the item id exist or not
	  4. Create BOM window and set Revsion rule
	  5. Set Top Bomline
	  6. Iterate all bom child line

    @param command line argument Item Id,User,Password,Group,Revsion Rule,Folder Path.
    @return int
*/

int ITK_user_main(int argc,char *argv[]) 
{
	ResultStatus rStatus;
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
	char cExportLog[500];   
	char cNotExportLog[500]; 
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

		if(argc!=7)
		{
			cout<<" Please enter correct arguments.."<<endl;
			cout<<" e.g mass_download_datasets_bvr.exe -u=infodba -p=infodba -g=dba -item_id=001 -rev_rule=\"Any Status; No Working\" -folder_path=\"E:\Files\""<<endl;
			return ITK_ok;
		}
		
		if(!cItem_id.empty())
		{
			rStatus=ITK_init_module(u.c_str(),p.c_str(),g.c_str());			
			cout<<" Login Successful"<<endl;
			cout<<" Trying to find Item "<<cItem_id.c_str()<<endl;
			int iNumOfItem = 0;
			scoped_smptr<tag_t> tItemTagList;
			string sAttValue;
			scoped_smptr<char> objType;
			sAttValue = "item_id="+cItem_id;
			rStatus=ITEM_find_items_by_string(sAttValue.c_str(),&iNumOfItem,&tItemTagList);
			for (int i = 0; i < iNumOfItem; i++)
			{
				WSOM_ask_object_type2(tItemTagList[i],&objType);
				cout<<" Object Type is ::" << objType.get() <<endl;
				if(tc_strcmp(objType.get(),"H4_Hon_Part_Type")==0)
				//if(tc_strcmp(objType.get(),"A2H4_Hon_Part")==0)
				{
					tItem = tItemTagList[i];
					cout<<" H4_Hon_Part_Type Object Type found"<<endl;
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

				pFile.open(cExportLog,ios::out);
				pFile2.open(cNotExportLog,ios::out);

				pFile<<"Item ID","Rev","Name","Status","Action","Lastest Rev Avaialble","Rev","Status","Action";

				pFile<<"Item ID","Rev","Name","Status","Reason";

				rStatus=BOM_create_window(&tWindow); 
				cout<<" Finding Revision Rule  "<<cRevRule.c_str()<<endl;
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

					cout<<" Please Enter correct revision rule Error"<<endl;
					return ITK_ok;
				}

				rStatus= BOM_set_window_config_rule( tWindow, tRule ); 
				cout<<" Revision Rule Set "<<endl;
				rStatus = BOM_save_window(tWindow);

				rStatus=BOM_set_window_top_line(tWindow,tItem,NULLTAG,NULLTAG,&tBomLine);  
				rStatus=BOM_line_ask_all_child_lines(tBomLine,&iNumberOfchild,&tBomChildren);  
				cout<<" Number of child to is : "<<iNumberOfchild<<endl;
				
				rStatus=BOM_line_look_up_attribute("bl_revision",&iAttribute);  
				rStatus = BOM_line_ask_attribute_tag(tBomLine,iAttribute,&tTop_Asm);
				rStatus = ITEM_ask_rev_id2(tTop_Asm,&rev_id);   

				if(rev_id != ITK_ok)
				{
					cout<<" Exporting dataset of Top BomLine "<<rev_id.get()<<endl;
					export_dataset(tTop_Asm,"H4_Source_File");
					export_dataset(tTop_Asm,"IMAN_reference");
					
					if(iNumberOfchild>0)
					{
						rStatus=bom_sub_child(tBomChildren.get(),iNumberOfchild);

					}
				}
				else
				{
					cout<<" item is not released"<<endl;
				}
				rStatus = BOM_close_window(tWindow);
				pFile2.close();
				pFile.close();
			}
			else
			{
				cout<<" Item not found Error : "<<endl;
			}			

		}
		else
		{
			cout<<" Enter -item_id= ?"<<endl;
			exit(0);
		}
	}
	catch(IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}
	return ITK_ok;
}

/**
    * Recursive function to Iterate all bom child line
	*
	* This function will iterate all child line of bom recursively. 
	* Also call export_dataset function to export dataset
	*
    @param tag_t BomLine, int number Of child .
    @return int.
*/
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
	char cChildName[WSO_name_size_c+1];  
	char cChildDes[WSO_name_size_c+1];   
	char cChildType[WSO_name_size_c+1];  
	scoped_smptr<char> rev_id;

	try{
		rStatus=BOM_line_look_up_attribute("bl_revision",&iAttribute);

		for(j=0;j<iNumberOfchild;j++)
		{
			rStatus = BOM_line_ask_attribute_tag(tBomChildren[j],iAttribute,&tChild); 
			rStatus = ITEM_ask_rev_id2(tChild,&rev_id); 
			if(rev_id != ITK_ok)
			{
				rStatus = WSOM_ask_name(tChild,cChildName); 
				rStatus = WSOM_ask_description(tChild,cChildDes);
				rStatus = WSOM_ask_object_type(tChild,cChildType);
				rStatus = BOM_line_ask_all_child_lines(tBomChildren[j],&iNumberOfSubChild,&tSubChilds);  
				cout<<" Exporting dataset of BomLine "<<cChildName <<" "<< rev_id.get()<<endl;
				export_dataset(tChild,"H4_Source_File");
				export_dataset(tChild,"IMAN_reference");
				if(iNumberOfSubChild>0)
				{	
					rStatus=bom_sub_child(tSubChilds.get(),iNumberOfSubChild); 
				}

			}
			else
			{
				//write_csv_file2(tBomChildren[j]);
			}

		}
	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}

	return ITK_ok;
}

/**
    * Export the dataset named reference attached with specific relation.
	*
	* This function export all namereference attached with dataset with specific relation .
	* We are exporting all dataset attached with Item Revsion with H4_source_File and Iman_Reference Relation and
	* Also we are exporting all dataset attached with Drawing with H4_source_File.
	
    @param tag_t Item, string Relation.
    @return void.
*/
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
	char cDestFilePath[500];  
	boolean isExported = false;

	try{
		if(tItem!= NULLTAG)
		{
			if(tc_strcmp(cRelation.c_str(),"H4_Source_File")==0)
				rStatus=GRM_find_relation_type("H4_Source_File",&tRelationTag);
			if(tc_strcmp(cRelation.c_str(),"IMAN_reference")==0)
				rStatus=GRM_find_relation_type("IMAN_reference",&tRelationTag);

			rStatus=GRM_list_secondary_objects_only(tItem,tRelationTag,&iCount,&tSecObjlist);
			
			cout<<" ====================Exporting dataset start============================"<<endl;
			cout<<" secondary objects count:"<< iCount <<endl;
			for(i=0;i<iCount;i++)
			{
				rStatus=WSOM_ask_object_type2(tSecObjlist[i],&cObjectType);	
				cout<<" ObjectType: "<<cObjectType.get()<<endl;

				if(tc_strcmp(cObjectType.get(),"MISC")==0)
				{
					continue;
				}

				if(tc_strcmp(cObjectType.get(),"H4_Drawing_Type")==0)
				{
					cout<<"Calling function for export_latest_rel_data H4_Drawing_Type.."<<endl;
					export_latest_rel_data(tSecObjlist[i]);
				}
				
				iFail=AE_ask_dataset_named_refs(tSecObjlist[i],&iFoundDataSet,&tRefObjectList);
				for(j=0;j<iFoundDataSet;j++)
				{
					rStatus = IMF_ask_original_file_name(tRefObjectList[j],cRefObjName);
					cout<<" RefObjName: "<<cRefObjName<<endl;
					tc_strcpy(cDestFilePath," ");
					tc_strcpy(cDestFilePath, cFolderPath.c_str());
					tc_strcat(cDestFilePath,"\\");
					tc_strcat(cDestFilePath,cRefObjName);
					cout<<" refName: "<<cObjectType.get()<<endl;
					rStatus=AE_ask_dataset_datasettype(tSecObjlist[i],&tDatasetType);
					
					if( tDatasetType != NULLTAG )
					{
						rStatus = AE_ask_datasettype_refs (tDatasetType, &ref_count, &ref_list);

						for (k = 0; k < ref_count; k++)
						{ 
							iFail=AE_export_named_ref(tSecObjlist[i],ref_list[k],cDestFilePath);
						}
					}
					
					if(iFail==ITK_ok)
					{
						isExported =true;
						cout<<" Data Exported succesffuly!!"<<endl;

					}
					else
					{
						EMH_ask_error_text(iFail,&cError);
						cout<<" Error ::!!"<<cError.get()<<endl;
					}
				}

			}
			cout<<" ====================Exporting dataset End============================"<<endl;
			//write_csv_file(tItem,isExported);
		}
		else
		{
			cout<<" not able to find Item tag"<<endl;
		}

	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}


}

/**
    * Get the latest release item rev tag from the Item.
	*
	* This function will iterate through all Item revsions of Item.
	* Check the property(release_status_list) of each Item revison.
	* If item revision has Released status then it send that item revsion send to export_dataset Function.
	* export_dataset function export data at given path. 
	
    @param tag_t Item.
    @return void.
*/
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
		cout<<" No of HON Drawing Type Revision found: "<<iCount<<endl;
		for (i = iCount-1; i >= 0; --i)
		{			
			rStatus=AOM_UIF_ask_value(tItemRevList[i],"release_status_list",&cPropValue);

			if(tc_strcmp(cPropValue.get(),"Released")==0)
			{
				cout<<" No of HON Drawing Type status is released.."<<endl;
				cout<<" getting Item Rev ID"<<endl;
				rStatus=ITEM_ask_rev_id2(tItemRevList[i],&cItemName);
				cout<<"  Item is Released..."<<cItemName.get()<<endl;
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

/**
    * Create a folder where dataset is going to export.
	*
	* This function will create folder at given folder path
	* folder path will be input from user. 
	
    @param string folder path.
    @return int.
*/
int create_export_folder(string cFolder_Path)
{
	int check;
	try{
		check = _mkdir(cFolder_Path.c_str());
		if(!check)
			cout<<" Directory created sucessfully!!"<<endl;
		else
			cout<<" Directory already exist!!"<<endl;
	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}
	return check;
}

/**
    * Write .CSV file which will contain exported Item data.
	*
	* Get the property from item tag and write in csv file.
	* This file contain data of exported Item 
	
    @param Item Rev tag.
    @return void.
*/
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
	cout<<" File csv file wrting started for..."<<endl;
	try
	{	
		rStatus = ITEM_ask_item_of_rev(tRevTag,&tItem);

		rStatus = ITEM_ask_id2(tItem,&cItem_Id);

		rStatus = ITEM_ask_rev_id2(tRevTag,&cItem_Rev);

		rStatus=ITEM_ask_name2(tItem,&cItem_Name);

		rStatus = AOM_UIF_ask_value(tRevTag,"release_status_list",&cPropValue);

		rStatus = ITEM_ask_latest_rev(tItem,&cLatestRevTag);

		rStatus = ITEM_ask_rev_id2(cLatestRevTag,&cLatestRev);
	
		if(isExported)
		{
			cAction = "Exported";
		}
		else
		{
			cAction = "Not Exported";
		}
		if (tc_strcmp(cItem_Rev.get(),cLatestRev.get())==0)
		{

			/*fprintf(pFile,"\n%s,%s,%s,%s,%s,%s,%s,%s,%s",
			cItem_Id,cItem_Rev,cItem_Name,cPropValue,cAction,"NA","NA","NA","NA");*/
			pFile << cItem_Id.get(),cItem_Rev.get(),cItem_Name.get(),cPropValue.get(),cAction.get(),"NA","NA","NA","NA";
		}
		else
		{
			/*fprintf(pFile.get(),"\n%s,%s,%s,%s,%s,%s,%s,%s,%s",
				cItem_Id,cItem_Rev,cItem_Name,cPropValue,cAction,"Y",cLatestRev,"Working","Skipped");*/
				pFile << cItem_Id.get(),cItem_Rev.get(),cItem_Name.get(),cPropValue.get(),cAction.get(),"Y",cLatestRev.get(),"Working","Skipped";
		}
	}
	catch(const IFail &ex)
	{
		cout<<"\n uncepted exception captured "<<ex.getMessage()<< "please report errors.\n"<<endl;
	}

}

/**
    * Write .CSV file contain bomline which does not exported.
	*
	* Get the property from bomline tag using BOM_line_look_up_attribute 
	* and write in csv file.This file contain data which is not loaded 
	* by revsion rule(e.g ? will be there in structure manager) 
	
    @param bomline tag.
    @return void.
*/
void write_csv_file2(tag_t tBomLine)
{

	ResultStatus rStatus;
	scoped_smptr<char> cItem_Id;
	scoped_smptr<char> cItem_Rev;
	scoped_smptr<char> cItem_Name;

	int iAttribute=0;

	cout<<" File csv file2 wrting started for..."<<endl;

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

