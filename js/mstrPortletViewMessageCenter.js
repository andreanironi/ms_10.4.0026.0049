// Receiving / sending cross-domain message

if (typeof(mstrPortletViewMessageCenter) == 'undefined') { //avoid duplicated define

mstrPortletViewMessageCenter = new Object();

//master prompt slected options
mstrPortletViewMessageCenter.slectedOptionMsg = null;

/**
 * onmessage event handler.
 * Process the message from MSTR portlet iframe (i.e. MSTR Web).
 */
mstrPortletViewMessageCenter.receiveMessage = function(e) {
	
	mstrPortletViewMessageCenter.log('received message: ' + e.data);

	if (!e.data) return;

	var data = e.data.split(mstrPortlet.MSG_DELIMITER1);
	if (!data || data.length != 2) return;
	
	var type = data[0];
	var msg = data[1];
	
	switch (type)
	{
	case mstrPortlet.MSG_TYPE_PORTLET_ID:
		//send portlet id to master portlets
		mstrPortletViewMessageCenter.sendPortletIdToAllMaster();
		break;
	case mstrPortlet.MSG_TYPE_PORTLETS:
		var master = unescape(mstrPortletViewMessageInfo.getValue(msg, 'id'));
		mstrPortletViewMessageCenter.sendPortletInfoToMaster(master);
		break;
	case mstrPortlet.MSG_TYPE_SENDTO_DEFAULT:
		var master = unescape(mstrPortletViewMessageInfo.getValue(msg, 'id'));
		var url = unescape(mstrPortletViewMessageInfo.getValue(msg, 'url'));
		mstrPortletViewMessageCenter.sendObjectToDefaultTarget(master, url);
		break;
	case mstrPortlet.MSG_TYPE_SENDTO:
		var master = unescape(mstrPortletViewMessageInfo.getValue(msg, 'id'));
		var target = unescape(mstrPortletViewMessageInfo.getValue(msg, 'target'));
		var url = unescape(mstrPortletViewMessageInfo.getValue(msg, 'url'));
		mstrPortletViewMessageCenter.sendObjectToTarget(master, target, url);
		break;
	case mstrPortlet.MSG_TYPE_SENDTO_BY_TITLE:
		var targetTitle = unescape(mstrPortletViewMessageInfo.getValue(msg, 'targetTitle'));
		var url = unescape(mstrPortletViewMessageInfo.getValue(msg, 'url'));
		var project = ''; //TQMS 442263. we don't know the project of the origion portlet.
		mstrPortletViewMessageCenter.sendObjectToTargetByTitle(project, targetTitle, url);
		break;
	case mstrPortlet.MSG_TYPE_PROMPT_OPTION:
		//the first message from master prompt. save selected options.
    	mstrPortletViewMessageCenter.slectedOptionMsg = msg;
		break;
	case mstrPortlet.MSG_TYPE_PROMPT_ANSWER:
		//the second message from master prompt.
		var master = unescape(mstrPortletViewMessageInfo.getValue(mstrPortletViewMessageCenter.slectedOptionMsg, 'id'));
		if (master) {
			//send the prompt answers to target portlet(s).
	   		mstrPortletViewMessageCenter.sendPromptAnswers(msg);
	   		//send portlets to the master prompt.
	   		mstrPortletViewMessageCenter.sendPortletInfoToMaster(master);
	   		//send a message to the master prompt to update the slected options.
	   		mstrPortletViewMessageCenter.sendSelectedOptionsBack(master);
   		}
   		//clean saved message
   		mstrPortletViewMessageCenter.slectedOptionMsg = null;
		break;
	case mstrPortlet.MSG_TYPE_URL_LINK:
		var master = unescape(mstrPortletViewMessageInfo.getValue(msg, 'id'));
		var target = mstrPortletViewMessageInfo.getValue(msg, 'target');
		if (target) target = unescape(target);
		var url = unescape(mstrPortletViewMessageInfo.getValue(msg, 'url'));
		mstrPortletViewMessageCenter.sendUrlLink(master, target, url);
		break;
	case mstrPortlet.MSG_TYPE_DRILL:
		var master = unescape(mstrPortletViewMessageInfo.getValue(msg, 'id'));
		var target = mstrPortletViewMessageInfo.getValue(msg, 'target');
		if (target) target = unescape(target);
		var params = unescape(mstrPortletViewMessageInfo.getValue(msg, 'params'));
		mstrPortletViewMessageCenter.sendDrill(master, target, params);
		break;
	case mstrPortlet.MSG_TYPE_SEND_AS_FILTER:
		var master = unescape(mstrPortletViewMessageInfo.getValue(msg, 'id'));
		var target = unescape(mstrPortletViewMessageInfo.getValue(msg, 'target'));
		var units = unescape(mstrPortletViewMessageInfo.getValue(msg, 'units'));
		//send message to target report
		mstrPortletViewMessageCenter.sendSelectedItemsAsFilter(master, target, units);
		break;
	default:
		mstrPortletViewMessageCenter.log('unsupported type: ' + type);
	}

}

mstrPortletViewMessageCenter.log = function(msg) {
	if (typeof(console) != "undefined") {
		console.log('[portal] ' + msg);
	}
}

/**
 * Send a message via window.postMessage.
 * @param {window} target The target window which receives the message.
 * @param {string} msg The message.
 * @param {string} origin The origin uri of the current window.
 */
mstrPortletViewMessageCenter.sendMessage = function(target, msg, origin) {
	if (target && mstrPortlet.CROSS_DOMAIN_MESSAGE_ENABLED) {
		target.postMessage(msg, origin);
	}
}

/**
 * Sends a message to each master folder and master history list portlet.
 * The message includes the id of each master portlet.
 */
mstrPortletViewMessageCenter.sendPortletIdToAllMasterFolderAndHistoryList = function() {
	
    mstrPortletViewMessageCenter.log('send portlet id to each master folder and master history list.');
    
	//get portlets
	var mstrIframes = mstrPortletView.getPortlets();
    
	//send message to each master portlet
	for(var i=0; i < mstrIframes.length; i++) {
		var isMaster = mstrIframes[i].getAttribute('isMaster') && mstrIframes[i].getAttribute('isMaster') == 'true';
		var evt = mstrPortlet.getEvent(mstrIframes[i].getAttribute('src'));
		if (isMaster && (evt == mstrPortlet.EVENT_FOLDER || evt == mstrPortlet.EVENT_HISTORY)) {
			var msg = mstrPortletViewMessageInfo.serializePortletIdMessage(mstrIframes[i].getAttribute('id'));
			mstrPortletViewMessageCenter.sendMessage(mstrIframes[i].contentWindow, msg, mstrIframes[i].src);
    	}
    }
}

/**
 * Sends a message to each master portlet.
 * The message includes the id of each master portlet.
 */
mstrPortletViewMessageCenter.sendPortletIdToAllMaster = function() {
	
    mstrPortletViewMessageCenter.log('send portlet id to each master.');
    
	//get portlets
	var mstrIframes = mstrPortletView.getPortlets();
    
	//send message to each master portlet
	for(var i=0; i < mstrIframes.length; i++) {
		var isMaster = mstrIframes[i].getAttribute('isMaster') && mstrIframes[i].getAttribute('isMaster') == 'true';
		if (isMaster) {
			var msg = mstrPortletViewMessageInfo.serializePortletIdMessage(mstrIframes[i].getAttribute('id'));
			mstrPortletViewMessageCenter.sendMessage(mstrIframes[i].contentWindow, msg, mstrIframes[i].src);
    	}
    }
}

/**
 * Sends a message to the specified master portlet or to all master portlets.
 * The message includes information of all portlets which have the same projcet as the master portlet.
 * @param {string} master The id of the master porltet iframe. this function sends to all master if it is -1.
 */
mstrPortletViewMessageCenter.sendPortletInfoToMaster = function(master) {
	if (!master) return;

	if (master == '-1') {
		mstrPortletViewMessageCenter.sendPortletInfoToAllMaster();
		return;
	}
	
	mstrPortletViewMessageCenter.log('send portlet info message to master ' + master);
	
	var masterIframe = document.getElementById(master);
	if (masterIframe == null) return;
	
	var isMaster = masterIframe.getAttribute('isMaster') && masterIframe.getAttribute('isMaster') == 'true';
	if (!isMaster) return;

	var project = masterIframe.getAttribute('project');
	if (!project || project == '') return;

	var mstrIframes = mstrPortletView.getPortlets();
	//serialize the portlets whose project property equals to the given project to a message string.
	var msgPortlets = mstrPortletViewMessageInfo.serializeRelatedPortletsMessage(project, mstrIframes);
	var msg = mstrPortletViewMessageInfo.serializePortletInfoMessage(masterIframe, msgPortlets);
	mstrPortletViewMessageCenter.sendMessage(masterIframe.contentWindow, msg, masterIframe.src);
}

/**
 * Sends a message to each master portlet.
 * The message includes the information of portlets which have the same projcet as the master portlet.
 */
mstrPortletViewMessageCenter.sendPortletInfoToAllMaster = function() {
	mstrPortletViewMessageCenter.log('send portlet info message to all master.');
    
	var mstrIframes = mstrPortletView.getPortlets();
	
    //serialize the portlets to a message list. The messages are catalogued by projects.
    var msgList = mstrPortletViewMessageInfo.serializeRelatedPortletsMessageList(mstrIframes);
	
	//send message to each master portlet
	for(var i=0; i < mstrIframes.length; i++) {
		var isMaster = mstrIframes[i].getAttribute('isMaster') && mstrIframes[i].getAttribute('isMaster') == 'true';
		if (!isMaster) continue;
		
		var project = mstrIframes[i].getAttribute('project');
		if (!project || project == '') continue;
		
		var msg = mstrPortletViewMessageInfo.serializePortletInfoMessage(mstrIframes[i], msgList[project]);
		mstrPortletViewMessageCenter.sendMessage(mstrIframes[i].contentWindow, msg, mstrIframes[i].src);
    }
	
}

/**
 * Sends a message to each master prompt/report/rwd portlet.
 * The message includes the information of portlets which have the same projcet as the master portlet.
 */
mstrPortletViewMessageCenter.sendPortletInfoToAllMasterPrompts = function() {
	mstrPortletViewMessageCenter.log('send portlet info message to each master prompt/report/rwd.');
    
	var mstrIframes = mstrPortletView.getPortlets();
	
    //serialize the portlets to a message list. The messages are catalogued by projects.
    var msgList = mstrPortletViewMessageInfo.serializeRelatedPortletsMessageList(mstrIframes);
	
	//send message to each master portlet
	for(var i=0; i < mstrIframes.length; i++) {
		var isMaster = mstrIframes[i].getAttribute('isMaster') && mstrIframes[i].getAttribute('isMaster') == 'true';
		var evt = mstrPortlet.getEvent(mstrIframes[i].getAttribute('src'));
		if (isMaster && (evt == mstrPortlet.EVENT_RUN_REPORT || evt == mstrPortlet.EVENT_EXECUTE_RWD)) {
			var project = mstrIframes[i].getAttribute('project');
			if (msgList[project]) {
				var msg = mstrPortletViewMessageInfo.serializePortletInfoMessage(mstrIframes[i], msgList[project]);
				mstrPortletViewMessageCenter.sendMessage(mstrIframes[i].contentWindow, msg, mstrIframes[i].src);
        	}
    	}
    }
}

/**
 * Send an object to the default target portlet.
 * @param {string} master The id of the origin master portlet.
 * @param {string} url The query part of the http url of the object.
 */
mstrPortletViewMessageCenter.sendObjectToDefaultTarget = function(master, url) {

	var masterIframe = document.getElementById(master);
    if (masterIframe == null) return;
	
	var targetTitle = masterIframe.getAttribute('tportlet');
	
	if (targetTitle == null || targetTitle == '') {
    	//find first available target title
    	var project = masterIframe.getAttribute('project');
    	if (project != null && project != '') {
    		var portletsStr = cookiesUtils.getCookie(document, escape(project));
    		if (portletsStr != null) {
    			var portlets = portletsStr.split('"');
	    		if (portlets.length > 0) targetTitle = portlets[0];
	    	}
    	}
    	//if not found, send to new window
    	if (targetTitle == '') targetTitle = 'New Window';
    }
    
    if (targetTitle == 'New Window') {
    	var message = url.substring(url.indexOf("?")+1);
    	var newURL = mstrPortlet.getNewSrc(masterIframe.src, masterIframe, message);
    	mstrPortletViewMessageCenter.sendObjectToNewWindow(newURL);
    } else {
    	var project = masterIframe.getAttribute('project');
    	mstrPortletViewMessageCenter.sendObjectToTargetByTitle(project, targetTitle, url);
    }
    
}

/**
 * Send an object to a new window.
 * @param {string} url The query part of the http url of the object.
 */
mstrPortletViewMessageCenter.sendObjectToNewWindow = function(url) {
	// Open in new window (use current session).
    // Remove 'iframe=true' and 'src=....'
    var newUrl = url.replace('iframe=true', ''); 
    newUrl = newUrl.replace(/src=[^&]*&/g, ''); 
    newUrl = newUrl.replace(/&src=[^$]*$/g, '');
    window.open(newUrl);
}

/**
 * Send an object to the target portlet.
 * @param {string} project The project name of the sending portlet.
 * @param {string} targetTitle The title of the target portlet.
 * @param {string} url The query part of the http url of the object.
 */
mstrPortletViewMessageCenter.sendObjectToTargetByTitle = function(project, targetTitle, url) {
	if (!url || url == '') return;
	
    var targetIframe = null;
    var mstrIframes = mstrPortletView.getPortlets();
	if (targetTitle && targetTitle != '') {
    	//find the portlet by title and project
    	for(var i=0; i < mstrIframes.length; i++){
			if(mstrIframes[i].getAttribute('title') == targetTitle) {
			    targetIframe = mstrIframes[i];
			    //TQMS 442263. skip project checking if we don't know the project of the original portlet.
			    if(project == '' || targetIframe.getAttribute('project') == project) {
				    break;
				}
			}
		}
    }
    
    if(targetIframe == null) {
        var str = mstrPortletViewDescriptors['portletNotFound'];
		alert(str.replace('{0}', targetTitle));
        return;
    }
    //TQMS 442263. skip project checking if we don't know the project of the original portlet.
    if(project != '' && targetIframe.getAttribute('project') != project) {
       var str = mstrPortletViewDescriptors['portletDiffProject'];
       alert(str.replace('{0}',targetIframe.getAttribute('title')));
       return;
    }
    if(targetIframe.getAttribute('src') == "") {
        var str = mstrPortletViewDescriptors['portletDesignMode'];
        alert(str.replace('{0}',targetIframe.getAttribute('title')));
        return false;
    }
    
    mstrPortletViewMessageCenter.sendObjectToIframe(targetIframe, url);
    
}

/**
 * Send an object to the target portlet.
 * @param {string} target The id of the target portlet.
 * @param {string} url The query part of the http url of the object.
 */
mstrPortletViewMessageCenter.sendObjectToTarget = function(master, target, url) {
	if (!url || url == '') return;
	
	if (target == '-1') {
    	//send to new window
    	var masterIframe = document.getElementById(master);
	    if (masterIframe == null) return;
    	var message = url.substring(url.indexOf("?")+1);
    	var newURL = mstrPortlet.getNewSrc(masterIframe.src, masterIframe, message);
    	mstrPortletViewMessageCenter.sendObjectToNewWindow(newURL);
    	return;
    }
	
    var targetIframe = document.getElementById(target);
    
    if(targetIframe == null) {
        var str = mstrPortletViewDescriptors['portletNotFound'];
		alert(str.replace("{0}", target));
        return;
    }
    
    mstrPortletViewMessageCenter.sendObjectToIframe(targetIframe, url);
}

/**
 * Send an object to the target iframe.
 * @param {iframe} targetIframe The target iframe.
 * @param {string} url The query part of the http url of the object.
 */
mstrPortletViewMessageCenter.sendObjectToIframe = function(targetIframe, url) {
	
    if(targetIframe.getAttribute('src') == "") {
        var str = mstrPortletViewDescriptors['portletDesignMode'];
        alert(str.replace("{0}",targetIframe.getAttribute('title')));            
        return;
    }
    
    // message is the query part of the http url
    var message = url.substring(url.indexOf("?")+1);
    message = mstrPortlet.filter(message);
    
    targetIframe = mstrPortlet.getTargetIframe(targetIframe, message);
    
    if (targetIframe != null) {
	    var newSrc = mstrPortlet.updatePortletSrc(targetIframe, message);
	    cookiesUtils.setCookie(window.document, targetIframe.getAttribute('id'), message);
	}
	//TODO else, show message like "It is not allowed to send to Master Portlet."
	
}

/**
 * Send prompt answer to the target portlet iframe(s).
 * @param {string} promptAnswerXML The prompt answer.
 */
mstrPortletViewMessageCenter.sendPromptAnswers = function(promptAnswerXML) {
	
	if (!promptAnswerXML) return;
	
	//deserialize selected options from message.
	var selectedOptions = mstrPortletViewMessageInfo.deserializePromptSlectedOptions(mstrPortletViewMessageCenter.slectedOptionMsg);

	promptAnswerXML = unescape(promptAnswerXML).replace(/\+/g,' ');
	promptAnswerXML = unescape(promptAnswerXML).replace(/"/g,'\'');
	
    var len = selectedOptions.targetPortlets.length;
    for (var i = 0; i < len; i++) {
        mstrPortletViewMessageCenter.sendPromptAnswer(selectedOptions.targetPortlets[i], promptAnswerXML, selectedOptions);
    }
}

/**
 * Send prompt answer to the target portlet iframe.
 * @param {iframe} targetIframe The target iframe.
 * @param {string} promptAnswerXML The prompt answer.
 */
mstrPortletViewMessageCenter.sendPromptAnswer = function(targetIframe, promptAnswerXML, selectedOptions) {

	if (!targetIframe) return;
	
	var isSendSlaveReport = selectedOptions.isSendSlaveReport && selectedOptions.slaveReport && selectedOptions.slaveReport.name;
	
	if (!isSendSlaveReport) {
		// If only send prompt answers, only send to Report and RWD portlets
		var evt = mstrPortlet.getEvent(targetIframe.getAttribute('src'));
	    var isTargetReportOrRWD = evt == mstrPortlet.EVENT_RUN_REPORT
			|| evt == mstrPortlet.EVENT_EXECUTE_RWD
			|| evt == mstrPortlet.EVENT_OPEN_OIVM;
	    if (!isTargetReportOrRWD) return;
	    
	    // Donnot send to Master Prompt
	    var pageName = mstrPortlet.getPageName(targetIframe.getAttribute('src'));
		var isMasterPrompt = pageName == 'portlet2portletMasterPrompt';
		if (isMasterPrompt) return;
	}
    
	var newURL;

	if (isSendSlaveReport) {
		var message;
	    if (selectedOptions.slaveReport.type && selectedOptions.slaveReport.type == 3) {
	    	message = targetIframe.getAttribute("report_url").concat("&reportID=" + selectedOptions.slaveReport.id );
	    } else {
	    	message = targetIframe.getAttribute("doc_url").concat("&documentID=" + selectedOptions.slaveReport.id);
	    }
		cookiesUtils.setCookie(window.document, targetIframe.getAttribute('id'), message);
		
		var oldURL = targetIframe.getAttribute("src");
		oldURL =  oldURL.replace(/reportName=.*?&/, "" ).replace(/reportID=.*?&/,"").replace(/documentName=.*?&/, "" ).replace(/documentID=.*?&/,""); 
		newURL = mstrPortlet.getNewSrc(oldURL, targetIframe, message);
	
    } else {
    	//If we first send with slave report, then send without slave report, the second message should succeed.
		newURL = targetIframe.getAttribute("src");
		var message = cookiesUtils.getCookie(window.document, targetIframe.getAttribute('id'));
		if (message) {
			var oldURL = newURL;
			oldURL =  oldURL.replace(/reportName=.*?&/, "" ).replace(/reportID=.*?&/,"").replace(/documentName=.*?&/, "" ).replace(/documentID=.*?&/,""); 
			newURL = mstrPortlet.getNewSrc(oldURL, targetIframe, message);
		}
    }
    
    var targetPostFormInput = window.document.createElement('input');
    targetPostFormInput.type = 'hidden';
	targetPostFormInput.name = 'promptsAnswerXML';
	targetPostFormInput.value = promptAnswerXML;
	
    //target post form
    var targetPostForm = window.document.createElement('form');
    targetPostForm.method = 'post';
    targetPostForm.target = targetIframe.getAttribute('id');
    targetPostForm.appendChild(targetPostFormInput);
    var actionString = newURL;
    if ((actionString.lastIndexOf('&')+1) != actionString.length) {
    	actionString = actionString.concat("&");
    }
    
    targetPostForm.action = actionString.concat("PromptAnswerMode=1");
    
    window.document.body.appendChild(targetPostForm);
    
    targetPostForm.submit();

}

/**
 * Send selected options back to the master prompt.
 * @param {string} master The id of the master iframe.
 */
mstrPortletViewMessageCenter.sendSelectedOptionsBack = function(master) {
	mstrPortletViewMessageCenter.log('send selected options back to master ' + master);
	
	var masterIframe = document.getElementById(master);
	if (masterIframe == null) return;

	var msg = mstrPortletViewMessageInfo.serializePromptOptionsMessage(mstrPortletViewMessageCenter.slectedOptionMsg);
	mstrPortletViewMessageCenter.sendMessage(masterIframe.contentWindow, msg, masterIframe.src);
}

//--------------------
// master report p2p
//--------------------

/**
 * get default target portlet of the master portlet iframe
 * @return "New Window" if sent to new window, or the iframe if found, or null if not found.
 */
mstrPortletViewMessageCenter.getDefaultTargetIframe = function(masterIframe) {
	
	if (masterIframe == null) return null;
	
	var targetIframe = null;
	
	var targetTitle = masterIframe.getAttribute('tportlet'); //TODO use id instead of title
	if (targetTitle == 'New Window') {
		return "New Window";
	}
	else {
	    var iframes = mstrPortletView.getPortlets();
	    if (targetTitle && targetTitle != '') {
		    for(var i=0;i<iframes.length;i++){
		        if(iframes[i].getAttribute('title') == targetTitle) {
		            targetIframe = iframes[i];
		            if(iframes[i].getAttribute('project') == masterIframe.getAttribute('project')) {
		                break;
		            }
		        }
		    }
		}
	}
	
	return targetIframe;
}

/**
 * return target portlet iframe.
 * @param {iframe} masterIframe the master iframe.
 * @param {string} target the id of target iframe. If it is null, return the default target of master iframe.
 * @return "New Window" if sent to new window, or the iframe if found, or null if not found.
 */
mstrPortletViewMessageCenter.getTargetPortletIframe = function(masterIframe, target) {

	var targetIframe = null;
	
	if (target) {
   		targetIframe = document.getElementById(target);
	}
	else {
		//default target portlet
    	targetIframe = mstrPortletViewMessageCenter.getDefaultTargetIframe(masterIframe);

    	if (targetIframe == 'New Window') {
			return targetIframe;
		}
	}

    if(targetIframe == null) {
        var str = mstrPortletViewDescriptors['portletNotFound'];
		alert(str.replace("{0}", target ? target : masterIframe.getAttribute('tportlet')));
        return null;
    }
    if(targetIframe.getAttribute('project') != masterIframe.getAttribute('project')) {
        var str = mstrPortletViewDescriptors('portletDiffProject');
        alert(str.replace("{0}",targetIframe.getAttribute('title')));        
        return null;
    }
    if(targetIframe.getAttribute('src') == "") {      
        var str = mstrPortletViewDescriptors('portletDesignMode');
        alert(str.replace("{0}",targetIframe.getAttribute('title')));            
        return null;
    }
	
	return targetIframe;
	
}

/**
 * Send a url link to the target portlet.
 * @param {string} target The id of the target portlet. null: default target; -1: new window.
 * @param {string} url The url link.
 */
mstrPortletViewMessageCenter.sendUrlLink = function(master, target, url) {
	
	mstrPortletViewMessageCenter.log('send url link to target ' + target);
	
	if (!url || url == '') return;
	
	var masterIframe = document.getElementById(master);
	if (masterIframe == null) return;
	
	var sendToNewWindow = false;
	var targetIframe = null;
	
	if (target == "-1") {
		sendToNewWindow = true;
	}
	else {
		targetIframe = mstrPortletViewMessageCenter.getTargetPortletIframe(masterIframe, target);
		if(targetIframe == null) return;
		
		if (targetIframe == "New Window") {
			sendToNewWindow = true;
		}
		else {
			if (targetIframe.getAttribute('isMaster') && targetIframe.getAttribute('isMaster') == 'true') {
	    		return;
	    	}
		}
    }
    
    var newURL = url;
    var method = "GET"; //in case the url does not allow POST, such as www.google.com
    
    if (url.toLowerCase().indexOf("http://") != 0 && url.toLowerCase().indexOf("https://") != 0) {
    	//send an object such as report, document, folder, etc.
    	if (sendToNewWindow) {
    		newURL = mstrPortlet.getNewSrc(masterIframe.src, masterIframe, url);
    	}
    	else {
    		newURL = mstrPortlet.getNewSrc(targetIframe.src, targetIframe, url);
    	}
    	method = "POST"; //in case the url has long parameters, such as linkAnswers
    }
    //else send an url link
    
    //create and submit form
	var oForm = document.createElement("FORM");
    
    oForm.name = "dynamic_form";
    oForm.action = newURL;
    
    if (sendToNewWindow) {
		oForm.target = "_blank";
	}
	else {
		//NOTE donnot use target since if it is null then targetIframe is the default target!
		oForm.target = targetIframe.id ? targetIframe.id : targetIframe.name;
	}
	oForm.method = method;
	
	document.body.appendChild(oForm);
    oForm.submit();
    
}

/**
 * Send dill reslut report to the target portlet.
 * @param {string} master The id of the master iframe.
 * @param {string} target The id of the target iframe. If it is 
 *       null then send to the default target of the master.
 *       If it is '-1' then send to new window.
 * @param {string} params The query parameters of the drill event.
 */
mstrPortletViewMessageCenter.sendDrill = function(master, target, params) {

	mstrPortletViewMessageCenter.log('drill to target ' + target);
	
	var masterIframe = document.getElementById(master);
	if (masterIframe == null) return;
	
	var sendToNewWindow = false;
	var targetIframe = null;
	
	if (target == "-1") {
		sendToNewWindow = true;
	}
	else {
		targetIframe = mstrPortletViewMessageCenter.getTargetPortletIframe(masterIframe, target);
		if (targetIframe == "New Window") {
			sendToNewWindow = true;
		}
		else {
	    	if(targetIframe == null) return;
	    	if (targetIframe.getAttribute('isMaster') && targetIframe.getAttribute('isMaster') == 'true') {
	    		return;
	    	}
    	}
    }
    
    var message = params.substring(params.indexOf("?")+1);
    var oParams = mstrPortlet.getParameterTable(message);
	
    var url = masterIframe.getAttribute('src');
	oParams['action'] = url.indexOf('?') >= 0 ? url.substring(0, url.indexOf('?')) : url;
	
	//set mstrwid. NOTE For drill, do not use target iframe id, or there will be job expire error.
	oParams['mstrwid'] = masterIframe.id ? masterIframe.id : masterIframe.name;
	
	//use target's page name (to use target's transforms)
	var currentPageName = mstrPortlet.getPageName(masterIframe.getAttribute('src'));
	var targetPageName = sendToNewWindow ? 'report' : mstrPortlet.getPageName(targetIframe.getAttribute('report_url'));
	
	if (targetPageName == null || targetPageName == '') {
		targetPageName = 'report';
	}
	
	if (currentPageName != null && currentPageName != '') {
		var myRegExp = new RegExp('\\.' + currentPageName, "g");
    	if (oParams['events']) oParams['events'] = oParams['events'].replace(myRegExp, '.' + targetPageName);
		if (oParams['src']) oParams['src'] = oParams['src'].replace(myRegExp, '.' + targetPageName);
	}
	
	//create and submit form
	var oForm = mstrPortlet.createFormFromParams(oParams);
	if (sendToNewWindow) {
		oForm.target = '_blank';
	}
	else {
		//NOTE donnot use target since if it is null then targetIframe is the default target!
		oForm.target = targetIframe.id ? targetIframe.id : targetIframe.name;
	}
	
	oForm.method = 'POST';
	
	document.body.appendChild(oForm);
    oForm.submit();
    
}

/**
 * send selected items as filter to target portlet.
 * delegate to target iframe. 
 * this is also called directly by mstrPortlet.sendAsFilter 
 * if cross-domain messaging is not supported by the browser, e.g. IE6, IE7.
 */
mstrPortletViewMessageCenter.sendSelectedItemsAsFilter = function(master, target, units) {
	
	mstrPortletViewMessageCenter.log('send as filter to target ' + target);
	
	if (!master || master== "") return;
	if (!target || target == "") return;
	if (!units) return;
	
	var masterIframe = document.getElementById(master);
	if (masterIframe == null) return;
	
	var targetIframes;
	
	if (target == "-1") { //send to all
		targetIframes = mstrPortletView.getPortlets();
	}
	else {
		var targetIframe = mstrPortletViewMessageCenter.getTargetPortletIframe(masterIframe, target);
		if (targetIframe == null || targetIframe == "New Window") return;
		targetIframes = [targetIframe];		
	}
	
	var currentProject = masterIframe.getAttribute('project');
    for(var i = 0; i < targetIframes.length; i++){
        if (targetIframes[i] && targetIframes[i].getAttribute('project') == currentProject 
        		&& targetIframes[i].getAttribute('src') != "" ) {
	        if (mstrPortlet.CROSS_DOMAIN_MESSAGE_ENABLED) {
	        	//send via cross-domain messaging
	        	var msg = mstrPortletViewMessageInfo.serializeSendAsFilterMessage(units);
	        	mstrPortletViewMessageCenter.sendMessage(targetIframes[i].contentWindow, msg, targetIframes[i].src);
	        }
	        else {
	        	//direct call
	        	if (units && targetIframes[i].contentWindow.mstrPortletReport) {
   					targetIframes[i].contentWindow.mstrPortletReport.sendAsFilter(units);
   				}
   			}
        }
    }
}

}