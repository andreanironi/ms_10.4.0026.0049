/** 
 * Cookie Utils object 
 */
var cookiesUtils = new Object ();

/**
 * Sets a Cookie with the given name and value.
 * doc        The document object of the cookie.
 * name       The name of the cookie
 * value      The value of the cookie
 */
cookiesUtils.setCookie = function(doc, name, value) {

    doc.cookie = name + "=" + escape(value) + "; path=/";
}


/**
 * Gets the value of the specified cookie.
 * doc    The document object of the cookie.
 * name   The name of the desired cookie.
 *
 * Returns a string containing value of specified cookie,
 *   or null if cookie does not exist.
 */
cookiesUtils.getCookie = function (doc, name) {
    var dc = doc.cookie;
    var prefix = name + "=";
    var begin = dc.indexOf("; " + prefix);
    if (begin == -1) {
        begin = dc.indexOf(prefix);
        if (begin != 0) return null;
    } else {
        begin += 2;
    }
    var end = doc.cookie.indexOf(";", begin);
    if (end == -1) {
        end = dc.length;
    }
    return unescape(dc.substring(begin + prefix.length, end));
}

/**
 * Deletes the specified cookie.
 *
 * doc    The document object of the cookie.
 * name   The name of the cookie
 */
cookiesUtils.deleteCookie = function (doc, name) {
    if (this.getCookie(doc,name)) {
        doc.cookie = name + "=" + "; path=/; expires=Thu, 01-Jan-70 00:00:01 GMT";
    }
}
