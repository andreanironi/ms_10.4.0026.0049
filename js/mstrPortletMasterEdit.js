
var mstrPortletMasterEdit = new Object();

mstrPortletMasterEdit.buildTargetPortletDropDown = function() {
    if(mstrPortletMasterEdit.isTargetPortletDropDownBuilt) {
        return;
    }
    
    mstrPortletMasterEdit.isTargetPortletDropDownBuilt = true;
    
    var oSelects = document.getElementsByName(mstrPortletDescriptors['targetPortlet']);
    
    var dc = document.cookie;
    for(var i=0;i<oSelects.length;i++) {
        var oSelect = oSelects[i];
        var tportlet = oSelect.getAttribute('tportlet');
        var project = oSelect.getAttribute('project');
        if (project != null && project != '') {
	        var prefix = escape(project) + "=";
	        var begin = dc.indexOf(prefix);
	        if (begin != -1) {
	            var end = dc.indexOf(";", begin);
	            if (end == -1) {
	                end = dc.length;
	            }
	            var portlets = unescape(dc.substring(begin + prefix.length, end)).split('"');
	            
	            for(var j=0;j<portlets.length;j++) {
	            	var targetTitle = mstrPortlet.XSSDecode(portlets[j]);
	                if(tportlet == targetTitle) {
	                    oSelect.options[j] = new Option(targetTitle, targetTitle, false, true);
	                }
	                else {
	                    oSelect.options[j] = new Option(targetTitle, targetTitle);
	                }
	            }
	            if(tportlet == 'New Window') {
	                oSelect.options[j] = new Option(mstrPortletDescriptors['newWindow'], 'New Window', false, true);
	            }
	            else {
	                oSelect.options[j] = new Option(mstrPortletDescriptors['newWindow'], 'New Window');
	            }
	        }
        }
    }

    return true;
}

/**
 * Load Settings from portlet to portlet message
 */
mstrPortletMasterEdit.loadSettingsFromMessage = function(oForm)  {

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

mstrPortletMasterEdit.checkRadioButton = function(radios, value) {
    for(var i=0;i<radios.length;i++){
    if(radios[i].value == value) {
        radios[i].checked = 'true';
        return;
    }
    }
}

mstrPortletMasterEdit.selectPullDownMenuItem = function(options, value) {
    for(var i=0;i<options.length;i++){
    if(options[i].value == value) {
        options[i].selected = 'true';
        return;
    }
    }
}
