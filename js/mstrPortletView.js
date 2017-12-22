// The js runs in potal.

if (typeof(mstrPortletView) == 'undefined') { //avoid duplicated define

var mstrPortletView = new Object();

mstrPortletView.mstrPortlets = null;

/**
 * Get a collection of MicroStrategy iframes.
 * @param {boolean} useCache True - use cache if available, false - regenerate and update cache.
 * @return {Array} The iframe arry.
 */
mstrPortletView.getPortlets = function(useCache) {
	var cacheEmpty = mstrPortletView.mstrPortlets == null || mstrPortletView.mstrPortlets.length == 0;
    if (!useCache || cacheEmpty) {
        var iframes = document.getElementsByTagName("iframe");
	    mstrPortletView.mstrPortlets = new Array();
	    for (var i=0; i<iframes.length;i++) {
	    	try {
	    	    if (iframes.item(i).getAttribute("type") == mstrPortlet.PORTLET_TYPE) {
	                mstrPortletView.mstrPortlets[mstrPortletView.mstrPortlets.length] = iframes.item(i);
	            }
	        } catch(error) {
	              // do nothing
	        }
	    }
    }
    return mstrPortletView.mstrPortlets;
}

/**
 * Look up mstr portlets and save to cookie by projects.
 */
mstrPortletView.lookupPortlet = function() {
    
    var mstrIframes = mstrPortletView.getPortlets();
    if(mstrIframes.length == 0){
        return;
    }
    
    var portlets = new Object();
    for(var i=0; i < mstrIframes.length; i++) {
    	var project = mstrIframes[i].getAttribute('project');
        if(typeof portlets[project] == 'undefined') {
        	// in case the title contains \".
            portlets[project] = mstrPortlet.XSSEncode(mstrIframes[i].getAttribute('title'));
        }
        else {
        	// in case the title contains \".
            portlets[project] += '"' + mstrPortlet.XSSEncode(mstrIframes[i].getAttribute('title'));
        }
    }

    for(var project in portlets) {
    	if(typeof(portlets[project]) == 'undefined' || portlets[project] == '') {
    		cookiesUtils.deleteCookie(document, escape(project));
    	}
    	else {
        	document.cookie = escape(project) + "=" + escape(portlets[project]) + "; path=/";
        }
    }
}

/**
 * This function is called on page load.
 * It looks up mstr portlets and save to cookie by projects.
 * It also sends a message to each master portelt if the browser support cross-domain messaging.
 */
mstrPortletView.onload = function() {
	mstrPortletView.lookupPortlet();
	if (mstrPortlet.CROSS_DOMAIN_MESSAGE_ENABLED) {
		//send portlet info to master report/document/prompt portlets
		mstrPortletViewMessageCenter.sendPortletInfoToAllMasterPrompts();
   	}
}

/**
 * This function is called when portal receives a cross-document message.
 */
mstrPortletView.onmessage = function(e) {
	mstrPortletViewMessageCenter.receiveMessage(e);
}

/***********************************
 * Register global event listeners *
 ***********************************/

// Portlet Lookup
if(window.addEventListener) { // Firefox
    window.addEventListener('load', mstrPortletView.onload, false);
}
else if(window.attachEvent) { // IE
    window.attachEvent('onload', mstrPortletView.onload);
}

// Listen Cross-Document messaging event
if (typeof(window.postMessage) != 'undefined') {
    if(window.addEventListener) { // Firefox3+, Safari4+, Chrome 1+
        window.addEventListener('message', mstrPortletView.onmessage, false);
	}
    else if(window.attachEvent) { // IE8+, Opera9.5+
        window.attachEvent('onmessage', mstrPortletView.onmessage);
	}
}

}
