// Serializing / deserializing cross-domain message

if (typeof(mstrPortletViewMessageInfo) == 'undefined') { //avoid duplicated define

mstrPortletViewMessageInfo = new Object();

mstrPortletViewMessageInfo.serializePortletIdMessage = function(id) {
	return mstrPortlet.MSG_TYPE_PORTLET_ID + mstrPortlet.MSG_DELIMITER1
		+ 'id=' + escape(id);
}

/**
 * serialize a target portlet information
 */
mstrPortletViewMessageInfo.serializeTargetPortletMessage = function(mstrIframe) {
	var id = mstrIframe.getAttribute('id');
	var title = mstrIframe.getAttribute('title');
	var isMaster = mstrIframe.getAttribute('isMaster') && mstrIframe.getAttribute('isMaster') == 'true';
	var evt = mstrPortlet.getEvent(mstrIframe.getAttribute('src')); //FIXME what if drill to target and then send as filter? use targetIframe.location.href instead?
	var pageName = mstrPortlet.getPageName(mstrIframe.getAttribute('src'));
	var isMasterPrompt = pageName == 'portlet2portletMasterPrompt';
	
	return escape(id) + mstrPortlet.MSG_DELIMITER4 
				+ escape(title) + mstrPortlet.MSG_DELIMITER4
				+ isMaster + mstrPortlet.MSG_DELIMITER4
				+ escape(evt) + mstrPortlet.MSG_DELIMITER4
				+ isMasterPrompt;
}

/**
 * serialize the portlets whose project property equals to the given project.
 * @param {String} project.
 * @param {Array(IFrame)} mstrIframes.
 * @return {string} the message.
 */
mstrPortletViewMessageInfo.serializeRelatedPortletsMessage = function(project, mstrIframes) {
	var msgPortlets = '';
    for(var i = 0; i < mstrIframes.length; i++) {
    	if (mstrIframes[i].getAttribute('src') && mstrIframes[i].getAttribute('src') != ''
    		&& mstrIframes[i].getAttribute('project') == project) {
    		
	    	var msgPortlet = mstrPortletViewMessageInfo.serializeTargetPortletMessage(mstrIframes[i]);
	    	
	    	if(msgPortlets == '') {
	            msgPortlets = msgPortlet;
	        }
	        else {
	            msgPortlets += mstrPortlet.MSG_DELIMITER3 + msgPortlet;
	        }
        }
    }
	
	return msgPortlets;
}

/**
 * serialize the portlets to a message list. 
 * Each message in the list includes all the portlets which has the same project property.
 * @param {Array(IFrame)} mstrIframes.
 * @return {Object} the message list which is catalogued by project.
 */
mstrPortletViewMessageInfo.serializeRelatedPortletsMessageList = function(mstrIframes) {
	//TODO cache?
	var msgList = new Object();
    for(var i = 0; i < mstrIframes.length; i++) {
    	if (mstrIframes[i].getAttribute('src') && mstrIframes[i].getAttribute('src') != '') {
    		
    		var msgPortlet = mstrPortletViewMessageInfo.serializeTargetPortletMessage(mstrIframes[i]);
	    	
	    	var project = mstrIframes[i].getAttribute('project');
	    	if(typeof msgList[project] == 'undefined') {
	            msgList[project] = msgPortlet;
	        }
	        else {
	            msgList[project] += mstrPortlet.MSG_DELIMITER3 + msgPortlet;
	        }
        }
    }
	return msgList;   
}

/**
 * serialize the portlet info of master portlet.
 * @param {IFrame} masterIframe.
 * @param {String} msgPortlets.
 * @return {string} the message.
 */
mstrPortletViewMessageInfo.serializePortletInfoMessage = function(masterIframe, msgPortlets) {
	var masterId = masterIframe.getAttribute('id');
	var showSendto = masterIframe.getAttribute('sendto') && masterIframe.getAttribute('sendto').toLowerCase() == "true";
	var defaultTarget;
	var tportlet = masterIframe.getAttribute('tportlet');
	if (tportlet == 'New Window') {
		defaultTarget = '-1';
	}
	else {
		defaultTarget = mstrPortletViewMessageInfo.getIframeIdByTitle(tportlet);
	}
	var enableAll = masterIframe.getAttribute('enableall') && masterIframe.getAttribute('enableall').toLowerCase() == "true";
	
	return mstrPortlet.MSG_TYPE_PORTLETS + mstrPortlet.MSG_DELIMITER1 
			+ 'id=' + escape(masterId) + mstrPortlet.MSG_DELIMITER2 
			+ 'showSendto=' + showSendto + mstrPortlet.MSG_DELIMITER2 
			+ 'tportlet=' + escape(defaultTarget) + mstrPortlet.MSG_DELIMITER2 
			+ 'enableAll=' + enableAll + mstrPortlet.MSG_DELIMITER2 
			+ 'portlets=' + msgPortlets;
}

mstrPortletViewMessageInfo.serializePromptOptionsMessage = function(slectedOptionMsg) {
	return mstrPortlet.MSG_TYPE_PROMPT_OPTION + mstrPortlet.MSG_DELIMITER1
		+ slectedOptionMsg;
}

mstrPortletViewMessageInfo.serializeSendAsFilterMessage = function(selectedUnits) {
	return mstrPortlet.MSG_TYPE_SEND_AS_FILTER + mstrPortlet.MSG_DELIMITER1
		+ 'units=' + escape(selectedUnits);
}

/**
 * Deserialize the selected options of master prompt.
 * @param {string} msg the message from the master prompt portlet.
 * @return {object} the selected option object.
 */
mstrPortletViewMessageInfo.deserializePromptSlectedOptions = function(msg) {
	
	var selectedOptions = new Object();
	
	if (!msg || msg == '') return selectedOptions;
	
	selectedOptions.originPortletId = unescape(mstrPortletViewMessageInfo.getValue(msg, 'id'));
	var msgTarget = mstrPortletViewMessageInfo.getValue(msg, 'target');
	selectedOptions.targetPortlets = mstrPortletViewMessageInfo.deserializeTargetPortlets(selectedOptions.originPortletId, msgTarget);
	selectedOptions.isSendSlaveReport = mstrPortletViewMessageInfo.getValue(msg, 'sendSlaveReport') == 'true';
	if (selectedOptions.isSendSlaveReport) {
		selectedOptions.slaveReport = mstrPortletViewMessageInfo.deserializePromptSlaveReport(mstrPortletViewMessageInfo.getValue(msg, 'slaveReport'));
	}
	else {
		selectedOptions.slaveReport = null;
	}
	
	return selectedOptions;
}

/**
 * Return selected target slave portlets for the master portlet.
 * @param {string} master The id of the master portlet.
 * @param {string} target The id list of the target slave portlets. -1 means All.
 * @return {array} The target portlets.
 */
mstrPortletViewMessageInfo.deserializeTargetPortlets = function(master, target) {
	var targetPortlets = new Array();
	
	if (!target || target == '') return targetPortlets;
	
	if (target == '-1') {
	    //broadcast
		targetPortlets = mstrPortletViewMessageInfo.getAllOtherTargetPortlets(master);
	} else {
		//selected portlets
		var targets = target.split(mstrPortlet.MSG_DELIMITER3);
		for (var i = 0; i < targets.length; i++) {
			var targetId = unescape(targets[i]);
		    var targetIframe = document.getElementById(targetId);
		    if (targetIframe) {
				targetPortlets.push(targetIframe);
			}
		}
	}
	
	return targetPortlets;
}

/**
 * Return all available target slave portlets for the master portlet.
 * @param {string} master The id of the master portlet.
 * @return {array} The target portlets.
 */
mstrPortletViewMessageInfo.getAllOtherTargetPortlets = function(master) {

	var targetPortlets = new Array();	
	
	if (!master || master == '') return targetPortlets;
	
	var masterIframe = document.getElementById(master);
	if (masterIframe == null) return targetPortlets;
	
	var isMaster = masterIframe.getAttribute('isMaster') && masterIframe.getAttribute('isMaster') == 'true';
	if (!isMaster) return targetPortlets;

	var project = masterIframe.getAttribute('project');
	if (!project || project == '') return targetPortlets;

	var mstrIframes = mstrPortletView.getPortlets();
    for(var i = 0; i < mstrIframes.length; i++) {
    	if (mstrIframes[i] == masterIframe) continue;
    	var src = mstrIframes[i].getAttribute('src');
    	if (src && src != '' && mstrIframes[i].getAttribute('project') == project) {
			targetPortlets.push(mstrIframes[i]);
        }
    }
    
    return targetPortlets;
}

/**
 * Deserialize the selected slave report.
 * @param {string} msg the message from master prompt.
 * @return {object} the slave report.
 */
mstrPortletViewMessageInfo.deserializePromptSlaveReport = function(msg) {
	var slaveReport = null;
	if (msg) {
		var tmpStr = msg.split(mstrPortlet.MSG_DELIMITER3);
		if (tmpStr && tmpStr.length == 3) {
			slaveReport = new Object();
			slaveReport.name = unescape(tmpStr[0]);
    		slaveReport.type = tmpStr[1]; 
			slaveReport.id = tmpStr[2];
		}
	}
	return slaveReport;
}

/**
 * Deserialize the selected units elements.
 * @param {string} msg the message from master report.
 * @return {Array} the selected units.
 */
mstrPortletViewMessageInfo.deserializeSelectedUnits = function(msg) {
	if (msg == null) {
		return null;
	}
	
	var selectedUnits = new Array();

	var unitsStrArray = msg.split(mstrPortlet.MSG_DELIMITER3);
    for (var i = 0; i < unitsStrArray.length; i++) {
    	var unitStr = unitsStrArray[i].split(mstrPortlet.MSG_DELIMITER4);
    	if (unitStr != null && unitStr.length >= 2) {
    		unitId = unescape(unitStr[0]);
    		elements = unescape(unitStr[1]);
    		selectedUnits[unitId] = elements;
    	}
    }
    
    return selectedUnits;
}

/**
 * Return the value of the key in the message.
 * @param {string} message the message.
 * @param {string} key the key.
 * @return {string} the value of the key.
 */
mstrPortletViewMessageInfo.getValue = function(message, key) {
    if (!message) return null;

    var msgs = message.split(mstrPortlet.MSG_DELIMITER2);
    for (var i = 0; i < msgs.length; i++) {
		if (msgs[i] != null) {
			var msg = msgs[i].split('=');
			if (msg != null && msg.length >= 2 && msg[0] == key) {
				return msg[1];
			}
		}
	}

	return null;
}

/**
 * Return the iframe id by its title.
 * @param {string} title The title of the iframe.
 * @return The id of the iframe if find, otherwise "-1".
 */
mstrPortletViewMessageInfo.getIframeIdByTitle = function(tile) {
   var iframes = mstrPortletView.getPortlets();
   for(var i=0; i < iframes.length; i++){
        if(iframes[i].getAttribute('title') == tile) {
            return iframes[i].getAttribute('id');
        }
   }
   return '-1';
}

}