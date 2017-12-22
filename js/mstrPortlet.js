/**
 * mstrPortlet is the object that provides functions to communicate between portlets.
 * This file includes functions run in both Portal page and MSTR portlet iframe.
 */

if (typeof(mstrPortlet) == 'undefined') { //avoid duplicated define

var mstrPortlet = new Object();

mstrPortlet.PORTLET_TYPE = "microstrategy.portlet.iframe";

mstrPortlet.EVENT_FOLDER = "2001";
mstrPortlet.EVENT_RUN_REPORT = "4001";
mstrPortlet.EVENT_HISTORY = "3018";
mstrPortlet.EVENT_SEARCH = "3040";
mstrPortlet.EVENT_PROJECT = "3010";
mstrPortlet.EVENT_RUN_HTMLDOC = "32001";
mstrPortlet.EVENT_EXECUTE_RWD = "2048001";
mstrPortlet.EVENT_OPEN_OIVM = "3135";
mstrPortlet.EVENT_EDIT_REPORT = "3005";
mstrPortlet.EVENT_EDIT_RWD = "3104";

//cross-domain message types
mstrPortlet.MSG_TYPE_PORTLET_ID = "1";
mstrPortlet.MSG_TYPE_PORTLETS = "2";
mstrPortlet.MSG_TYPE_SENDTO_DEFAULT = "3";
mstrPortlet.MSG_TYPE_SENDTO = "4";
mstrPortlet.MSG_TYPE_PROMPT_OPTION = "5";
mstrPortlet.MSG_TYPE_PROMPT_ANSWER = "6";
mstrPortlet.MSG_TYPE_SENDTO_BY_TITLE = "7";
mstrPortlet.MSG_TYPE_SEND_AS_FILTER = "8";
mstrPortlet.MSG_TYPE_DRILL = "9";
mstrPortlet.MSG_TYPE_URL_LINK = "10";

//cross-domain message delimiters
mstrPortlet.MSG_DELIMITER1 = '"';
mstrPortlet.MSG_DELIMITER2 = '&';
mstrPortlet.MSG_DELIMITER3 = ',';
mstrPortlet.MSG_DELIMITER4 = ':';

//MicroStrategy IFrames
mstrPortlet.mstrPortlets = null;

//If the browser supports cross-domain messaging or not.
mstrPortlet.CROSS_DOMAIN_MESSAGE_ENABLED = typeof(window.postMessage) != 'undefined';

/**
 * Get a collection of MicroStrategy iframes.
 */
mstrPortlet.getPortlets = function() {

	if (mstrPortlet.mstrPortlets == null || mstrPortlet.mstrPortlets.length == 0) {
	    
	    mstrPortlet.mstrPortlets = new Array();
	    try {
	    	var iframes = window.parent.document.getElementsByTagName("iframe");

	    	for (var i=0; i< iframes.length;i++) {
    			try {
        	    	if (iframes.item(i).getAttribute("type") == mstrPortlet.PORTLET_TYPE) {
	    				mstrPortlet.mstrPortlets[mstrPortlet.mstrPortlets.length]=iframes.item(i);
    	    		}
    	    	} catch (error) {
    	    		//In IE, if the type is not defined, it will throw an error. In this case, we want to ingore the error. 
    	    	}
            }
        } catch (error) {
     		//ignore error.
	    }
	}
	return mstrPortlet.mstrPortlets;
}

/**
 * Get the current MicroStrategy Iframe.
 */
mstrPortlet.getThisPortlet = function() {
	
	var iframes = mstrPortlet.getPortlets();
	var currentIframe = null;
	for(var i=0;i<iframes.length;i++){
	   if(iframes[i].contentWindow == window){
	      currentIframe = iframes[i];
	      break;
	   }
	}
	return currentIframe;
	
}

/**
 * Return the event from a query string (ex: evt=2001&src=mstrWeb....)
 */
mstrPortlet.getEvent = function(url) {
    var message = url.substring(url.indexOf("?")+1);
    var oParams = this.getParameterTable(this.filter(message));
    return oParams['evt'];
}

mstrPortlet.canSendToMaster = function(event) {
	return event == mstrPortlet.EVENT_FOLDER
			|| event == mstrPortlet.EVENT_RUN_REPORT 
    		|| event == mstrPortlet.EVENT_EXECUTE_RWD
    		|| event == mstrPortlet.EVENT_OPEN_OIVM;
}

/**
 * Get the target iframe based on the sent type.
 * If send a folder/report/doc, allow to send to all portlets.
 * Else, only allow to send to slave portlets.
*/
mstrPortlet.getTargetIframe = function(targetIframe, message) {

	var oParams = this.getParameterTable(this.filter(message));
    var sentEvent = oParams['evt'];
    var canSendToMaster = mstrPortlet.canSendToMaster(sentEvent);
    var isTargetMaster = targetIframe.getAttribute('isMaster') && targetIframe.getAttribute('isMaster') == 'true';
    
   	if (isTargetMaster && !canSendToMaster) {
   		return null; // Do nothing
   	}
   	else {
   		return targetIframe;
   	}
    
}

/**
 * sendToDefaultPortlet: This method takes an event. The event is optional for IE. 
 * content of the default portlet. This function is used by the transforms in package
 * com.microstrategy.plugins.portlet2portlet.transforms.
 */
mstrPortlet.sendToDefaultPortlet = function(e) {

    if(!e) e = window.event;
    e.cancelBubble = true;
    
    var oDom = getEventTarget(e);
    oDom = microstrategy.findAncestorWithTag(oDom, 'A');
    if(!oDom) return false;
    
    var url = oDom.href;
    return mstrPortlet.sendLinkToDefaultPortlet(url);
    
}

/**
 * This function send the link to the default target portlet.
 * This function is used by the transforms in package
 * com.microstrategy.plugins.portlet2portlet.transforms.contextmenus.
 * 
 * url: The string represents the url of an object.
 *
 */
mstrPortlet.sendLinkToDefaultPortlet = function(url) {
    
    if (!url) return false;
    
    if (mstrPortlet.CROSS_DOMAIN_MESSAGE_ENABLED) {
    	return mstrPortletPluginMessageCenter.sendObjectToDefaultTarget(url);
    }
    else {
    	//directly call parent window's js, may throw permission denied exception if in different domains
    	try {
    		var currentIframe = mstrPortlet.getThisPortlet();
    		if (!currentIframe) return true;
    		window.parent.mstrPortletViewMessageCenter.sendObjectToDefaultTarget(currentIframe.getAttribute('id'), url);
    	} catch(err) {
    	    alert(err);
    	}
    }
    
    return false;
    
}

/**
 * This function send the message to the target iframe. 
 * DONNOT remove this function since it may be called by cutomized master portlets. 
 * MSDL https://resource.microstrategy.com/MSDZ/MSDL/921/docs/mergedProjects/websdk/topics/portalintgr/PI_Using_RSDoc_as_Master_Portlet.htm
 * targetTitle: The title of the target iframe.
 * message: The string represents the portlet to portlet message. It has the syntax of HTTP query.
 *
 */
mstrPortlet.sendTo = function (targetTitle, message) {
   
   if (mstrPortlet.CROSS_DOMAIN_MESSAGE_ENABLED) {
		mstrPortletPluginMessageCenter.sendObjectToPortletByTitle(targetTitle, message);
		return;
   }
   
   var iframes = mstrPortlet.getPortlets();
   var currentIframe = mstrPortlet.getThisPortlet();
   
   //find the target iframe. It must match the title and point to the same project.
   var targetIframe; 
   for(var i=0;i<iframes.length;i++){
      if(iframes[i].getAttribute('title') == targetTitle) {
         targetIframe = iframes[i];
         if(iframes[i].getAttribute('project') == currentIframe.getAttribute('project')) {
             break;
         }
      }
   }
   
   if (targetIframe) {
   	  mstrPortlet.updatePortletSrc(targetIframe, message);
   }

}

/**
 * This function takes iframe and message as arguments. 
 * It apply the message on the iframe. The parameter defined in the message, 
 * will override the same parameter defined in iframe source. 
 *
 * iframe:   The portlet iframe.
 * message:  The string represents the portlet to portlet message. 
 *           It has the syntax of HTTP query.
 */
mstrPortlet.updatePortletSrc = function (iframe, message) {
    //oldURL
    var oldURL = iframe.src; 
    var newURL = this.getNewSrc(oldURL, iframe, message);
    iframe.src=newURL;
}

/**
 * This function takes the old url, applies the messages on the old url, and returns a new url. 
 * oldURL: The old url.
 * iframe: The portlet iframe.
 * message:  The string represents the portlet to portlet message. 
 *           It has the syntax of HTTP query. 
 */
mstrPortlet.getNewSrc = function(oldURL, iframe, message) {
	
    var iQueryStart = oldURL.indexOf("?");
    var sBase;
    var sQuery='';
    var oParams = new Array();
    if (iQueryStart == -1) {
        sBase = oldURL;
    } else {
        sBase = oldURL.substring(0,iQueryStart);
        sQuery = oldURL.substring(iQueryStart+1);
        oParams = this.getParameterTable(this.filterOldQuery(sQuery)); //TQMS 428197
    }
    var oNewParams = this.getParameterTable(this.filter(message));
    for (var n in oNewParams) {
        oParams[n]=oNewParams[n];
    }

    //replace the src parameter with the target portlet's 'src' attribute.
    var sUpdateString = '';
    switch (oParams['evt']) {
        case '2001':
            sUpdateString = iframe.getAttribute('folder_url');
            break;
        case '4001':
            sUpdateString = iframe.getAttribute('report_url');
            break;
        case '3018':
            sUpdateString = iframe.getAttribute('history_url');
            break;
        case '3040':
            sUpdateString = iframe.getAttribute('search_url');
            break;
        case '3010':
            sUpdateString = iframe.getAttribute('project_url');
            break;
        case '32001':
            sUpdateString = iframe.getAttribute('html_doc_url');
            break;
        case '2048001':
        case '3115':
            sUpdateString = iframe.getAttribute('doc_url');
            break;
    }

    var oUpdateTable;

    oUpdateTable = this.getParameterTable(sUpdateString);

    for (var n in oUpdateTable) {
        oParams[n] = oUpdateTable[n];
    }

    if(oUpdateTable['src'] == null) {
        oParams['src'] = '';
    }

    var newURL = sBase+'?';
    for (var n in oParams) {
        newURL+=n+'='+oParams[n]+'&';
    }
    
    return newURL;
}


/**
 * This method take a query string (ex: evt=2001&src=mstrWeb....), and build
 * a parameter table.  
 */
mstrPortlet.getParameterTable = function(sQuery) {
    var oArray = new Array();
    
    if (!sQuery || sQuery=='') return oArray;
    
    var end = -1;
    var arrayOfStrings = sQuery.split('&');
    for (var i = 0;i<arrayOfStrings.length;i++) {
        var nameValue=arrayOfStrings[i].split('=');
        if (nameValue[0] && nameValue[0]!='' && nameValue[1] && nameValue[1]!='') {
            oArray[nameValue[0]]=nameValue[1];	
        }
    }
    return oArray;
}

/**
 * This method filters out unwanted parameters which are generated by the p2p message from the old query string.
 * sQuery:     The original query string.
 * Return:     The filtered query.
 */ 
mstrPortlet.filterOldQuery = function(sQuery) {

    if (!sQuery || sQuery=='') return '';

	var oMap = this.getParameterTable(sQuery);
    var newQuery = '';
    
    //following parameters will be removed from the old query
    var keys = ["messageID", "currentViewMedia", "reportDesignMode", "visMode",
    			"executionMode", "rwDesignMode", "objectType", "objectID", 
    			"events", "elementsPromptAnswers", "valuePromptAnswers", "linkAnswers",
    			"reportName", "folderName", "documentName", "systemFolder", "folderType"];

	for (var name in oMap) {
		var filter = false;
		for (var n in keys) {
			if (name == keys[n]) {
				filter = true;
				break;
			}
		}
        if (!filter) {
            newQuery += name + "=" + oMap[name] + "&";
        }
    }

    //remove the last '&'
    if (newQuery.length>0) return newQuery.substring(0,newQuery.length-1);
    else return newQuery;
}

/**
 * This method filters out unwanted parameters from the message string.
 * message:    The original message string.
 * Return:     The filtered message.
 */ 
mstrPortlet.filter = function(message) {

    if (!message) return '';

    var oMap = this.getParameterTable(message);
    var newMessage = '';
    
    //only following parameters will be added to the new query
    var keys = ["evt", "folderID", "reportID", "documentID", "reportViewMode", "events", 
    			"messageID", "visMode", "currentViewMedia", "reportDesignMode",
    			"executionMode", "rwDesignMode", "objectType", "objectID", 
    			"elementsPromptAnswers", "valuePromptAnswers", "linkAnswers"];

    for (var n in keys) {
        if (oMap[keys[n]]) {
            newMessage += keys[n] + "=" + oMap[keys[n]] + "&";
        }
    }

    //remove the last '&'
    if (newMessage.length>0) return newMessage.substring(0,newMessage.length-1);
    else return newMessage;
}


/** 
 * Sets the document domain to the highest level in order to communicate across different
 * machines. If the domain is "some.abc.com", this function will set the document's domain 
 * to "abc.com"
 */
mstrPortlet.liftDomain = function() {
	
    if (mstrPortlet.CROSS_DOMAIN_MESSAGE_ENABLED) return;
    
    var noCountrySuffixs = ["edu","com","gov","int","org","net","mil"];
    var sDomain = document.domain; 
    var domainArray = document.domain.split(".");
    var domainLevel=3;
    for (var n in noCountrySuffixs) {
        if (domainArray[domainArray.length-1] == noCountrySuffixs[n]) {
            domainLevel=2;
            break;
        }
    }

    if (domainArray.length >= domainLevel) {
        var sNewDomain = "";
        for (var i = domainArray.length-domainLevel; i < domainArray.length; i++) {
            sNewDomain+=domainArray[i];
            if (i != domainArray.length-1) sNewDomain+=".";
        }
        if (document.domain!=sNewDomain) { 
          mstrPortlet.originalDomain = document.domain;
          document.domain= sNewDomain;
	}
    }
}

//reset domain
mstrPortlet.resetDomain = function() {
    
    if (mstrPortlet.originalDomain) {
	document.domain = mstrPortlet.originalDomain; 
    }
}

//Gets the session state from sessionIFrame, and open the target iframeID with src plus the session state. 
mstrPortlet.openIFrame = function(iframeID, src) {
	
	//The session frame
	var iframe = document.getElementById("sessionIFrame"+iframeID);
	var response = iframe.contentWindow.mstrTaskResponse;
	
	if (!response) 
	{
		//unexpected content returned by task
		return;
	}
	if (response.statusCode != 200)
	{
		//unsuccessful		
		alert(response.errorMsg+". ["+response.errorCode+"]");
		
		return;
		
	} else {
	
		//successful
		var iframeDoc = iframe.contentWindow.document;

		//The actual frame
		var iframe2 = document.getElementById(iframeID);

		//minimal state
		var sessionStateNode = iframeDoc.getElementById("minState");
		var sessionState = "";
		if (sessionStateNode!=null) {
			sessionState = sessionStateNode.getAttribute("v");
		}

		//Set cookie
		var sessionInfoNode = iframeDoc.getElementById("sessionInfo");
		var sessionInfo = "";
		if (sessionInfoNode!=null) {
			sessionInfo = sessionInfoNode.getAttribute("v");
		}

		//set cookie, no need to escape for session state
		cookiesUtils.setCookie(document, "S_"+iframeID, sessionState + "|" +sessionInfo);
		
		//check if cookie is enabled
		if (document.cookie == "") {
			alert("Cookies need to be enabled to use MicroStrategy Portlets!");			
		}
		
		iframe2.src = src+"&usrSmgr="+escape(sessionState);
		
	}
	
}

mstrPortlet.getOldSessions = function(iframeID) {
	
	var oldSessions = cookiesUtils.getCookie(document, 'S_'+iframeID);
	if (oldSessions!=null) {
		var states = oldSessions.split("|");
		if (states.length==2) {
			return "oldSession="+escape(states[0])+"&oldSessionInfo="+escape(states[1]);
		}
	} else {
		return "";
	} 
}

mstrPortlet.XSSEncode = function(str) {
	if (!str) return '';
	return str.replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/\"/g, '&quot;').replace(/\'/g, '&#039;');
}

mstrPortlet.XSSDecode = function(str) {
	if (!str) return '';
	return str.replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&quot;/g, '\"').replace(/&#039;/g, '\'');
}


/**
 * get page name from url
 */
mstrPortlet.getPageName = function(url) {

	if (url == null) return null;

	var message = url.substring(url.indexOf('?') + 1);
    var oParams = mstrPortlet.getParameterTable(message);
	var srcParam = oParams['src'];
	
	if (srcParam == null) return null;

	if (srcParam.indexOf('mstrWeb.') >= 0) {
		//mstrWeb.portlet2portletSlaveReport
		var idx1 = srcParam.indexOf('mstrWeb.') + 7;
		var idx2 = srcParam.indexOf('.', idx1 + 1);
		if (idx2 == -1) idx2 = srcParam.length;
		
		return srcParam.substring(idx1 + 1, idx2);
	}
	else if (srcParam.indexOf('Main.aspx.') >= 0) {
		//Main.aspx.portlet2portletSlaveReport
		var idx1 = srcParam.indexOf('Main.aspx.') + 9;
		var idx2 = srcParam.indexOf('.', idx1 + 1);
		if (idx2 == -1) idx2 = srcParam.length;
		
		return srcParam.substring(idx1 + 1, idx2);
	}
}

/**
 * This method take a parameter table, and build a query string (ex: evt=2001&src=mstrWeb....)
 */
mstrPortlet.getParameterString = function(oParams) {
	var result = '';
    for (var n in oParams) {   
        if(oParams[n]) {
            if(typeof oParams[n] == 'object') {
               for (var j = 0; j < oParams[n].length; j++){
                    if(oParams[n][j]) {
                    	result += '&' + n + '=' + oParams[n][j];
                    }
               }
            } else {
            	result += '&' + n + '=' + oParams[n];
            }
        }
    }
    return result;
}

/**
 * Creates a dynamic FORM element and inserts it into the body of the document.
 * @param {Object} oParams The parameters that are supposed to be submitted by the request.
 * @type HTMLElement
 * @return The newly created and inserted form.
 */
mstrPortlet.createFormFromParams = function(oParams) {
    //Add a new form
    var oNewForm = document.createElement("FORM");
    
    oNewForm.name = "dynamic_form";
    oNewForm.method = oParams['method']; delete oParams['method'];
    oNewForm.action = oParams['action']; delete oParams['action'];
    if(typeof oParams['target'] != "undefined") {oNewForm.target = oParams['target']; delete oParams['target'];}
    oNewForm.setAttribute('hasUpdateChanges', oParams['hasUpdateChanges']=='true');

    for (var n in oParams) {   
        if(oParams[n]) {
            if(typeof oParams[n] == 'object') {
            	for (var j = 0; j < oParams[n].length; j++) {
            		if(oParams[n][j]) mstrPortlet.createHiddenInput(oNewForm, n, oParams[n][j]);
            	}
            } else {
            	mstrPortlet.createHiddenInput(oNewForm, n, oParams[n]);
            }
        }
    }

    return oNewForm;

}

/**
 * Creates a new hidden input, populates it's value and inserts it into the supplied form. 
 * @param {HTMLElement} oForm The form that should contain the new input.
 * @param {String} name The name of the new input.
 * @param {String} value The value of the new input.
 * @type HTMLElement
 * @return The new hidden input.
 */
mstrPortlet.createHiddenInput = function (oForm, name, value) {
    var oNewItem = document.createElement("INPUT");
    oNewItem.type="HIDDEN";
    oNewItem.name = name;
    oNewItem.value = value;

    oForm.appendChild(oNewItem);
    return oNewItem;
}

}