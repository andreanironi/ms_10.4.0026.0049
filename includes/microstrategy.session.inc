<?php

/**
* @file
* MicroStrategy session builder file.
*/

//task names
define('MSTR_CREATE_SESSION_TASK', 'getSessionState');
define('MSTR_LOGOUT_TASK', 'logout');

/** Specifies a session flag to cancel all active jobs when the session is closed. */
define('DssXmlSessionCancelActiveJobsOnClose', 0x00000001);
/** Specifies a session flag to delete all read messages when session is closed. */
define('DssXmlSessionDeleteReadMsgsOnClose', 0x00000010);
/** Specifies a session flag to delete all unread messages when session is closed. */
define('DssXmlSessionDeleteUnreadMsgsOnClose', 0x00000100);

/**
 * Close sessions for all MicroStrategy pages and blocks.
 * @return.
 */
function microstrategy_close_all_sessions() {
  microstrategy_close_all_pages_sessions();
  microstrategy_close_all_blocks_sessions();
}

/**
 * Close session for a MicroStrategy page.
 * @param $task_base_url task base url.
 * @param $id id of MicroStrategy page.
 * @return TRUE if succeed, else FALSE.
 */
function microstrategy_close_page_session($id) {
  if (isset($id) && $id > 0) {
    $task_base_url = _microstrategy_get_page_task_url($id);
    $session_state = _microstrategy_get_page_session_state($id);
    _microstrategy_clear_page_session($id);
    $title = _microstrategy_get_page_title($id);
    return _microstrategy_close_session($task_base_url, $session_state, $title);
  }
}

/**
 * Close sessions for all MicroStrategy pages.
 * @return error message if an error occurs.
 */
function microstrategy_close_all_pages_sessions() {
  $pages = microstrategy_get_pages();
  if (is_array($pages) && !empty($pages)) {
    foreach ($pages as $page) {
      microstrategy_close_page_session($page->pid);
    }
  }
}

/**
 * Close session for a MicroStrategy block.
 * @param $id id of MicroStrategy block.
 * @return TRUE if succeed, else FALSE.
 */
function microstrategy_close_block_session($id) {
  if (isset($id) && $id > 0) {
    $task_base_url = _microstrategy_get_block_task_url($id);
    $session_state = _microstrategy_get_block_session_state($id);
    _microstrategy_clear_block_session($id);
    $title = _microstrategy_get_block_title($id);
    return _microstrategy_close_session($task_base_url, $session_state, $title);
  }
}

/**
 * Close sessions for all MicroStrategy blocks.
 * @return.
 */
function microstrategy_close_all_blocks_sessions() {
  $blocks = microstrategy_get_blocks();
  if (is_array($blocks) && !empty($blocks)) {
    foreach ($blocks as $block) {
      microstrategy_close_block_session($block->bid);
    }
  }
}

/**
 * Get/Create session from MicroStrategy Web Server
 * @return An object containing the minimum session state.
 */
function microstrategy_create_session($config) {

  $result = new stdClass();

  $task_url = $config['task_url'];
  $task_message_array = _microstrategy_build_create_session_task_message($config);

  $headers = _microstrategy_get_request_headers();
  $headers = _microstrategy_filter_request_headers($headers);

  //call task
  //TQMS 459564. Use cURL which supports NTLM authentication if cURL is enabled, else use drupal_http_request.
  if (function_exists('curl_version')) {
    $task_result = _microstrategy_curl_post($task_url, $headers, $task_message_array);
  }
  else {
    $task_message = _microstrategy_encode_task_message($task_message_array);
    $task_result = drupal_http_request($task_url . $task_message, $headers, 'POST');
  }

  if (isset($task_result->error)) {
    watchdog('microstrategy', 'Creating a session caused an error: %error', array('%error' => $task_result->error), WATCHDOG_WARNING);
    $result->error = t('Creating a session caused an error: %error', array('%error' => $task_result->error));
    $result->code = $task_result->code;
    return $result;
  }

  if (!_microstrategy_parse_session_message($task_result->data)) {
    $parse_error = microstrategy_message_get();
    $result->error = t('The response from %task task caused the following error: %error on line %line.',
        array('%task' => MSTR_CREATE_SESSION_TASK, '%error' => $parse_error->error_message, '%line' => $parse_error->error_line));
    watchdog('microstrategy', 'Creating a session caused an error: %error', array('%error' => $result->error), WATCHDOG_WARNING);
    return $result;
  }

  $response_message = microstrategy_message_get();

  if ($response_message->status_code != '200') {
    watchdog('microstrategy', 'Creating a session caused an error: %error', array('%error' => $response_message->error_message), WATCHDOG_WARNING);
    $result->error = t('Creating a session caused an error: %error', array('%error' => $response_message->error_message));
    $result->code = $response_message->status_code;
    return $result;
  }

  if (isset($config['ispage'])) {
    //if it's a microstrategy page
    _microstrategy_set_page_session($config['id'], $response_message->min_state, $response_message->session_info);
  }
  elseif (isset($config['isblock'])) {
    //if it's a microstrategy block
    _microstrategy_set_block_session($config['id'], $response_message->min_state, $response_message->session_info);
  }

  $result->min_state = $response_message->min_state;
  return $result;
}

function _microstrategy_curl_post($url, $headers = array(), $data = NULL) {

  $result = new stdClass();

  // create curl resource
  $ch = curl_init($url);

  //return the transfer as a string
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_POST, 1);
  //cURL will poll the server to see what methods it supports and pick the best one.
  curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_NTLM);
  //DON'T remove bellow line. Use system account (need SSPI eanbled) for NTLM authentication.
  curl_setopt($ch, CURLOPT_USERPWD, ":");
  curl_setopt($ch, CURLOPT_TIMEOUT, 30);
  curl_setopt($ch, CURLOPT_FAILONERROR, 1);
  curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);

  if (!$output = curl_exec($ch)) {
    $result->error = curl_error($ch);
  }

  // close curl resource to free up system resources
  curl_close($ch);

  if ($output) {
    //FIXME do we need to parse response?
    $result = _microstrategy_parse_curl_response($output);

    if (!isset($result->data)) {
      $result->error = $output;
    }

  }

  return $result;

}

/**
 * returns an array in the following format which varies depending on headers returned

        [0] => the HTTP error or response code such as 404
        [1] => Array
        (
            [Server] => Microsoft-IIS/5.0
            [Date] => Wed, 28 Apr 2004 23:29:20 GMT
            [X-Powered-By] => ASP.NET
            [Connection] => close
            [Set-Cookie] => COOKIESTUFF
            [Expires] => Thu, 01 Dec 1994 16:00:00 GMT
            [Content-Type] => text/html
            [Content-Length] => 4040
        )
        [2] => Response body (string)
 * @param $response
 * @return unknown_type
 */
function _microstrategy_parse_curl_response($response){

  $result = new stdClass();

  list($response_headers, $result->data) = explode("\r\n\r\n",$response,2);
  $response_header_lines = explode("\r\n",$response_headers);

  // first line of headers is the HTTP response code
  $http_response_line = array_shift($response_header_lines);
  if (preg_match('@^HTTP/[0-9]\.[0-9] ([0-9]{3})@',$http_response_line,
                 $matches)) {
      $result->code = $matches[1];
  }

  // put the rest of the headers in an array
  $response_header_array = array();
  foreach ($response_header_lines as $header_line) {
      list($header,$value) = explode(': ',$header_line,2);
      $response_header_array[$header] = $value;
  }

  $result->headers = $response_header_array;

  return $result;
}

/**
 * Close session for a MicroStrategy page or a MicroStrategy block.
 * @param $task_base_url task base url
 * @param $session_state the MicroStrategy session state.
 * @param $title title of the MicroStrategy page or block.
 * @return TRUE if succeed, else FALSE.
 */
function _microstrategy_close_session($task_base_url, $session_state, $title) {

  if (empty($session_state)) {
    return FALSE;
  }

  //build message
  $task_message_array = _microstrategy_build_logout_task_message($session_state);
  $task_message = _microstrategy_encode_task_message($task_message_array);

  $headers = _microstrategy_get_request_headers();
  $headers = _microstrategy_filter_request_headers($headers);

  //call task
  //TQMS 459564. Use cURL which supports NTLM authentication if cURL is enabled, else use drupal_http_request.
  if (function_exists('curl_version')) {
    $result = _microstrategy_curl_post($task_url, $headers, $task_message_array);
  }
  else {
    $task_message = _microstrategy_encode_task_message($task_message_array);
    $result = drupal_http_request($task_url . $task_message, $headers, 'POST');
  }

  //process result
  if (isset($result->error)) {
    $t_arg = array('%title' => $title, '%error' => $result->error);
    watchdog('microstrategy', 'Closing a session for %title caused an error: %error', $t_arg, WATCHDOG_WARNING);
    return FALSE;
  }

  //parse response xml string
  if (!_microstrategy_parse_session_message($task_result->data)) {
    //parse fail
    $parse_error = microstrategy_message_get();
    $error_msg = t('The response from %task task caused the following error: %error on line %line.',
        array('%task' => MSTR_LOGOUT_TASK, '%error' => $parse_error->error_message, '%line' => $parse_error->error_line));
    watchdog('microstrategy', 'Creating a session caused an error: %error', array('%error' => $error_msg), WATCHDOG_WARNING);
    return FALSE;
  }

  //check task call result
  $response_message = microstrategy_message_get();
  if ($response_message->status_code != '200') {
    $t_arg = array('%title' => $title, '%error' => $response_message->error_message);
    watchdog('microstrategy', 'Closing a session for %title caused an error: %error', $t_arg, WATCHDOG_WARNING);
    return FALSE;
  }

  return TRUE;

}

/**
 * Clear MicroStrategy page session which is saved drupal session.
 * @param $pid page id.
 * @return
 */
function _microstrategy_clear_page_session($pid) {
  //unset($_SESSION[MSTR_PAGE_SESSION_STATE . $pid]);
  //unset($_SESSION[MSTR_PAGE_SESSION_INFO . $pid]);
  _microstrategy_set_page_session($pid, "", "");
}

function _microstrategy_set_page_session($pid, $min_state, $session_info) {
  $_SESSION[MSTR_PAGE_SESSION_STATE . $pid] = $min_state;
  $_SESSION[MSTR_PAGE_SESSION_INFO . $pid] = $session_info;
}

function _microstrategy_get_page_session_state($pid) {
  $result = NULL;
  if (isset($_SESSION[MSTR_PAGE_SESSION_STATE . $pid])) {
    $result = $_SESSION[MSTR_PAGE_SESSION_STATE . $pid];
  }
  return $result;
}

function _microstrategy_get_page_session_info($pid) {
  $result = NULL;
  if (isset($_SESSION[MSTR_PAGE_SESSION_INFO . $pid])) {
    $result = $_SESSION[MSTR_PAGE_SESSION_INFO . $pid];
  }
  return $result;
}

/**
 * Clear MicroStrategy block session which is saved drupal session.
 * @param $bid block id.
 * @return
 */
function _microstrategy_clear_block_session($bid) {
  //unset($_SESSION[MSTR_BLOCK_SESSION_STATE . $bid]);
  //unset($_SESSION[MSTR_BLOCK_SESSION_INFO . $bid]);
  _microstrategy_set_block_session($bid, "", "");
}

function _microstrategy_set_block_session($bid, $min_state, $session_info) {
  $_SESSION[MSTR_BLOCK_SESSION_STATE . $bid] = $min_state;
  $_SESSION[MSTR_BLOCK_SESSION_INFO . $bid] = $session_info;
}

function _microstrategy_get_block_session_state($bid) {
  $result = NULL;
  if (isset($_SESSION[MSTR_BLOCK_SESSION_STATE . $bid])) {
    $result = $_SESSION[MSTR_BLOCK_SESSION_STATE . $bid];
  }
  return $result;
}

function _microstrategy_get_block_session_info($bid) {
  return $_SESSION[MSTR_BLOCK_SESSION_INFO . $bid];
}

/**
 * create an array represents the task message
 * @param $config
 * @return unknown_type
 */
function _microstrategy_build_create_session_task_message($config) {
  module_load_include('inc', 'microstrategy', 'includes/microstrategy.crypto');

  $credential_mapper_file = isset($config['credential_mapper_file']) ? $config['credential_mapper_file'] : NULL;
  $credential_mapper_function = isset($config['credential_mapper_function']) ? $config['credential_mapper_function'] : NULL;
  $credentials = _microstrategy_get_mapped_credentials($config['sso_namespace'], $credential_mapper_file, $credential_mapper_function);

  if (_microstrategy_use_client_session($config['authmode'])) {
    $uid = '';
    $pwd = '';
  }
  else {
    $crypto = new MicrostrategyCrypto();
    $uid = isset($credentials['logon']) ? $credentials['logon'] : '';
    $pwd = isset($credentials['password']) ? $credentials['password'] : '';
  }

  if (isset($config['ispage'])) {
    $old_session_state = _microstrategy_get_page_session_state($config['id']);
    $old_session_info = _microstrategy_get_page_session_info($config['id']);
  }
  elseif (isset($config['isblock'])) {
    $old_session_state = _microstrategy_get_block_session_state($config['id']);
    $old_session_info = _microstrategy_get_block_session_info($config['id']);
  }

  //default locale - current drupal locale
  $default_locale = _microstrategy_get_drupal_locale();

  $display_locale = _microstrategy_resolve_locale($credentials['display_locale'], $default_locale);
  $data_locale = _microstrategy_resolve_locale($credentials['data_locale'], $default_locale);
  $metadata_locale = _microstrategy_resolve_locale($credentials['metadata_locale'], $default_locale);
  $warehouse_data_locale = _microstrategy_resolve_locale($credentials['warehouse_data_locale'], $default_locale);

  $request = array(
    'taskId' => MSTR_CREATE_SESSION_TASK,
    'taskEnv' => 'xml',
    'taskContentType' => 'xml',
    'server' => $config['iserver'],
    'port' => $config['port'],
    'project' => $config['project'],
    'authMode' => $config['authmode'],
    'uid' => $uid,
    'pwd' => $pwd,
    'oldSession' => $old_session_state,
    'oldSessionInfo' => $old_session_info,
    'displayLID' => $display_locale,
    'dataLID' => $data_locale,
    'metadataLID' => $metadata_locale,
    'warehouseDataLID' => $warehouse_data_locale,
    'clientID' => ip_address(),
    'rws' => $config['working_set_size'],
    'sFlags' => _microstrategy_get_session_flags($config),
    'appType' => '25', // set application type to Portal

  );

  return $request;

}

function _microstrategy_get_session_flags($config) {
  $flags = 0;

  if ($config['cancel_pending_request']) {
    $flags |= DssXmlSessionCancelActiveJobsOnClose;
  }

  if ($config['remove_history_jobs'] == 0) { //No
  }
  elseif ($config['remove_history_jobs'] == 1) { //Yes
    $flags |= DssXmlSessionDeleteReadMsgsOnClose;
    $flags |= DssXmlSessionDeleteUnreadMsgsOnClose;
  }
  elseif ($config['remove_history_jobs'] == 2) { //ReadOnly
    $flags |= DssXmlSessionDeleteReadMsgsOnClose;
  }

  return $flags;
}

function _microstrategy_build_logout_task_message($session_state) {

  $request = array(
    'taskId' => MSTR_LOGOUT_TASK,
    'taskEnv' => 'xml',
    'taskContentType' => 'xml',
    'sessionState' => $session_state,
  );

  return $request;

}

/**
 * Return HTTP request header
 * @return array
 */
function _microstrategy_get_request_headers() {
  $headers = array();
  foreach ($_SERVER as $k => $v) {
    if (substr($k, 0, 5) == "HTTP_") {
      $k = str_replace('_', ' ', substr($k, 5));
      $k = str_replace(' ', '-', ucwords(strtolower($k)));
      $headers[$k] = $v;
    }
  }
  return $headers;
}

/**
 * Filter HTTP request header.
 * @param $headers the origin header
 * @return array the result header
 */
function _microstrategy_filter_request_headers($headers) {

  $excluded_headers = array(
    "accept",
    "accept-charset",
    "accept-encoding",
    "accept-language",
    "accept-ranges",
    "authorization",
    "cache-control",
    "connection",
    "cookie",
    "date",
    "host",
    "if-modified-since",
    "if-none-match",
    "user-agent",
    "content-length",
  );

  $new_headers = array();
  foreach ($headers as $k => $v) {
    if (!in_array(strtolower($k), $excluded_headers)) {
      $new_headers[$k] = $v;
    }
  }

  return $new_headers;

}

/**
 * Encode a message from an array for HTTP Post
 * ?taskId=getSessionState&taskEnv=xml&taskContentType=xml&server=VM-SERVER-XYE&...
 */
function _microstrategy_encode_task_message($data) {
  $encoded_message = "";
  foreach ($data as $key => $value) {
    if (isset($value)) {
      if ($encoded_message != "") {
        $encoded_message .= "&";
      }
      else {
        $encoded_message .= "?";
      }
      $encoded_message .= rawurlencode(trim($key)) . "=" . rawurlencode(trim($value)); //urlencode
    }
  }
  return $encoded_message;
}

/**
 * Convert a xml message into an array.
 * @param $xml_data the session xml string
 * @return TRUE if parsed successfully, ortherwise FALSE.
 *  Call microstrategy_message_get() to get the result.
 */
function _microstrategy_parse_session_message($xml_data) {

  $xml_parser = drupal_xml_parser_create($xml_data);
  xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, FALSE);
  xml_set_element_handler($xml_parser, '_microstrategy_message_tag_open', '_microstrategy_message_tag_close');
  xml_set_character_data_handler($xml_parser, '_microstrategy_message_cdata');

  if (!xml_parse($xml_parser, $xml_data, 1)) {
    //parse error
    $microstrategy_message = microstrategy_message_get();
    $microstrategy_message->error_message = xml_error_string(xml_get_error_code($xml_parser));
    $microstrategy_message->error_line = xml_get_current_line_number($xml_parser);
    microstrategy_message_set($microstrategy_message);
    return FALSE;
  }
  xml_parser_free($xml_parser);

  return TRUE;
}

/**
 * Store a copy of the $microstrategy_message object temporarily.
 *
 * @param $value
 *   Object
 * @return
 *   The most recently stored $microstrategy_message
 */
function microstrategy_message_set($value = NULL) {
  static $microstrategy_message;
  if ($value) {
    $microstrategy_message = $value;
  }
  return $microstrategy_message;
}

function microstrategy_message_get() {
  return microstrategy_message_set();
}

function _microstrategy_message_tag_open($parser, $tag, $attributes) {
  $microstrategy_message = microstrategy_message_get();
  if (!isset($microstrategy_message)) {
    $microstrategy_message = new stdClass();
  }
  $microstrategy_message->current_tag_contents = '';
  $microstrategy_message->last_open = $tag;
  switch ($tag) {
    case 'taskResponse':
      $microstrategy_message->status_code = $attributes['statusCode'];
      if ($attributes['statusCode'] != '200') {
        $microstrategy_message->error_message = $attributes['errorMsg'];
        $microstrategy_message->task_error_code = $attributes['taskErrorCode'];
      }
      else {
        unset($microstrategy_message->error_message);
        unset($microstrategy_message->task_error_code);
      }
      break;
    default:
      break;
  }
  microstrategy_message_set($microstrategy_message);
}

function _microstrategy_message_cdata($parser, $cdata) {
  $microstrategy_message = microstrategy_message_get();
  $microstrategy_message->current_tag_contents = $cdata;
  microstrategy_message_set($microstrategy_message);
}

function _microstrategy_message_tag_close($parser, $tag) {
  $microstrategy_message = microstrategy_message_get();
  switch ($tag) {
    case 'min-state':
      $microstrategy_message->min_state = trim($microstrategy_message->current_tag_contents);
      break;
    case 'max-state';
      $microstrategy_message->max_state = trim($microstrategy_message->current_tag_contents);
      break;
    case 'session-info':
      $microstrategy_message->session_info = trim($microstrategy_message->current_tag_contents);
      break;
    case 'project-id':
      $microstrategy_message->project_id = trim($microstrategy_message->current_tag_contents);
      break;
    default:
      break;
  }
  $microstrategy_message->current_tag_contents = '';
  microstrategy_message_set($microstrategy_message);
}