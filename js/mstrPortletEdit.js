/**
 * The object for portlet edit.
 */
var mstrPortletEdit = new Object();

/**
 * Load Settings from portlet to portlet message
 */
mstrPortletEdit.loadSettingsFromMessage = function(oForm)  {

    //parse message to array
    var oArray = new Array();
	
    var msg = mstrPortlet.XSSDecode(this.message);
    
    //no message
    if (msg ==null || msg == "") return;
	
    var arrayOfStrings = msg.split('&');
    
    for (var i = 0;i<arrayOfStrings.length;i++) {
        var nameValue=arrayOfStrings[i].split('=');
        oArray[nameValue[0]]=nameValue[1];    
    }
    
    var radios = oForm.elements[mstrPortletDescriptors['type']];
    var evt = oArray['evt'];
        
    switch(evt){
    case '2001':
        var folderID = oArray['folderID'];
        if(folderID == '') {
            return;
        }
        this.checkRadioButton(radios, mstrPortletDescriptors['folder']); 
        this.selectPullDownMenuItem(oForm.elements[mstrPortletDescriptors['folderNameType']].options, mstrPortletDescriptors['id']);
        oForm.elements[mstrPortletDescriptors['folderName']].value = mstrPortlet.XSSEncode(folderID);
        break;
    case '4001':
        var reportID = oArray['reportID'];
        var reportViewMode = oArray['reportViewMode'];
	    if(reportID == '' || reportViewMode == ''){
	        return;
	    }
	    this.checkRadioButton(radios, mstrPortletDescriptors['report']);
	    this.selectPullDownMenuItem(oForm.elements[mstrPortletDescriptors['reportNameType']].options, mstrPortletDescriptors['id']);
	    oForm.elements[mstrPortletDescriptors['reportName']].value = mstrPortlet.XSSEncode(reportID);
	    this.selectPullDownMenuItem(oForm.elements[mstrPortletDescriptors['reportViewMode']].options, reportViewMode);
	    break;
    case '32001':
        var documentID = oArray['documentID'];
    	if(documentID == '') {
    	    return;
	    }
	    this.checkRadioButton(radios, mstrPortletDescriptors['htmlDoc']);
	    this.selectPullDownMenuItem(oForm.elements[mstrPortletDescriptors['htmlDocNameType']].options, mstrPortletDescriptors['id']);
	    oForm.elements[mstrPortletDescriptors['htmlDocName']].value = mstrPortlet.XSSEncode(documentID);
	    break;
    case '2048001':
    case '3135':
        var documentID = oArray['documentID'];
	    if(documentID == '') {
	        return;
	    }
	    this.checkRadioButton(radios, mstrPortletDescriptors['doc']);
	    this.selectPullDownMenuItem(oForm.elements[mstrPortletDescriptors['docNameType']].options, mstrPortletDescriptors['id']);
	    oForm.elements[mstrPortletDescriptors['docName']].value = documentID;
	    break;
    
    }        
}

mstrPortletEdit.checkRadioButton = function(radios, value) {
    for(var i=0;i<radios.length;i++){
    if(radios[i].value == value) {
        radios[i].checked = 'true';
        return;
    }
    }
}

mstrPortletEdit.selectPullDownMenuItem = function(options, value) {
    for(var i=0;i<options.length;i++){
    if(options[i].value == value) {
        options[i].selected = 'true';
        return;
    }
    }
}


mstrPortletEdit.validateCredentials = function(login, pwd) {
       
   if (login == '') {alert(mstrPortletDescriptors['logonCannotBeEmpty']);return false;}
   if (pwd == '' ) {alert(mstrPortletDescriptors['passwordCannotBeEmpty']);return false;}
   return true;
 
};

mstrPortletEdit.validateContent = function(oForm) {

    if ((oForm.elements[mstrPortletDescriptors['portletTitle']]) && oForm.elements[mstrPortletDescriptors['portletTitle']].value.indexOf('"') >= 0) {
        alert(mstrPortletDescriptors['titleError']);
        return false;
    }

    var type;    
    var contentTypes = oForm.elements[mstrPortletDescriptors['type']];
    for (i = 0; i< contentTypes.length; i++) {
        if (contentTypes[i].checked) {
            type = contentTypes[i].value;
            break;
        }
    }

    if (type == mstrPortletDescriptors['report'] && oForm.elements[mstrPortletDescriptors['reportName']].value=='') { 
        alert(mstrPortletDescriptors['reportNameCannotBeEmpty']);
        return false;
    }
    if (type == mstrPortletDescriptors['folder'] && oForm.elements[mstrPortletDescriptors['folderName']].value =='') { 
        alert(mstrPortletDescriptors['folderNameCannotBeEmpty']);
        return false;
    }
    if (type == mstrPortletDescriptors['doc'] && oForm.elements[mstrPortletDescriptors['docName']].value =='') { 
        alert(mstrPortletDescriptors['docCannotBeEmpty']);
        return false;
    }
    if (type == mstrPortletDescriptors['htmlDoc'] && oForm.elements[mstrPortletDescriptors['htmlDocName']].value =='') { 
        alert(mstrPortletDescriptors['htmlDocCannotBeEmpty']);
        return false;
    }
    if (type == mstrPortletDescriptors['prompt'] && oForm.elements[mstrPortletDescriptors['promptedReportName']].value == '') {
        alert(mstrPortletDescriptors['reportNameCannotBeEmpty']);
        return false;
    }
    
    return true;
    
};

