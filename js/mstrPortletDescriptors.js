
if (typeof(mstrPortletDescriptors) == 'undefined') { //avoid duplicated define

var mstrPortletDescriptors = new Object();

//descriptors used in external javascript file mstrPortletEdit.loadSettingsFromMessage.

mstrPortletDescriptors['type'] = 'type';
mstrPortletDescriptors['folderNameType'] = 'folder_name_type';
mstrPortletDescriptors['folderName'] = 'folder_name';
mstrPortletDescriptors['reportName'] = 'report_name';
mstrPortletDescriptors['reportNameType'] = 'report_name_type';
mstrPortletDescriptors['reportViewMode'] = 'report_view_mode';
mstrPortletDescriptors['htmlDocNameType'] = 'htmldoc_name_type';
mstrPortletDescriptors['htmlDocName'] = 'htmldoc_name';
mstrPortletDescriptors['docNameType'] = 'doc_name_type';
mstrPortletDescriptors['docName'] = 'doc_name';
mstrPortletDescriptors['promptedReportName'] = 'prompt_name_type';
mstrPortletDescriptors['targetPortlet'] = 'default_target';

mstrPortletDescriptors['folder'] = 1;
mstrPortletDescriptors['report'] = 0;
mstrPortletDescriptors['htmlDoc'] = 5;
mstrPortletDescriptors['doc'] = 6;
mstrPortletDescriptors['prompt'] = 7;

mstrPortletDescriptors['id'] = 0; //MSTR_CNT_TYPE_IDENTIFIER_ID
mstrPortletDescriptors['name'] = 1; //MSTR_CNT_TYPE_IDENTIFIER_NAME

}