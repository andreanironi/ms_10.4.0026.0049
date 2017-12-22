<?php

/**
* @file
* MicroStrategy IFrame builder file.
*/

define('MSTR_NAMESPACE_KEY', 'mstrwid');
define('MSTR_IFRAME_TYPE', 'microstrategy.portlet.iframe');

/**
 * Build iframe HTML code.
 * @param $config the configuration array.
 * @return the iframe HTML code.
 */
function microstrategy_build_iframe_code($config) {

  $use_client_auth = _microstrategy_use_client_session($config['authmode']);
  if (!$use_client_auth) {
    module_load_include('inc', 'microstrategy', 'includes/microstrategy.session');
    $result = microstrategy_create_session($config);
    if (isset($result->error)) {
      return "<div>" . $result->error . "</div>";
    }
  }

  switch ($config['scrolling']) {
    case 1:
      $scrolling = 'yes';
      break;
    case 2:
      $scrolling = 'no';
      break;
    default:
      $scrolling = 'auto';
      break;
  }

  $data = array(
    'type' => MSTR_IFRAME_TYPE,
    'src' => _microstrategy_create_iframe_src($config, $use_client_auth, $result->min_state),
    'width' => $config['width'],
    'height' => $config['height'],
    'scrolling' => $scrolling,
    'frameborder' => '0',
    'title' => $config['title'],
    'id' => MSTR_PREFIX . $config['id'],
    'name' => MSTR_PREFIX . $config['id'],
    'report_url' => $config['report_url'],
    'folder_url' => $config['folder_url'],
    'doc_url' => $config['doc_url'],
    'html_doc_url' => $config['htmldoc_url'],
    'history_url' => $config['history_url'],
    'search_url' => $config['search_url'],
    'project_url' => $config['project_url'],
    'project' => $config['project'],
  );

  //master
  if (isset($config['ismaster']) && $config['ismaster']) {
    $data['sendto'] = $config['show_sendto'] ? "true" : "false";
    $data['tportlet'] = $config['default_target'];
    $data['enableAll'] = $config['enable_all_for_send_as_filter'] ? "true" : "false";
    $data['isMaster'] = 'true';
  }

  return _microstrategy_encode_iframe_message($data);
}

/**
 * build iframe source url
 * @param $config
 * @return string: baseurl?evt=&folderID=&src=&mstrwid=&...
 */
function _microstrategy_create_iframe_src($config, $use_client_auth, $session_state) {

  $src = $config["base_url"] . "?";

  $map = _microstrategy_collect_iframe_parameters($config);

  //custom parameter mapper
  if (!empty($config['parameter_mapper_file']) && !empty($config['parameter_mapper_function'])) {

    $file = './'. drupal_get_path('module', 'microstrategy') ."/custom_plugin/" . $config['parameter_mapper_file'];

    if (is_file($file)) {
      require_once $file;
    }

    if (function_exists($config['parameter_mapper_function'])) {
      $map = $config['parameter_mapper_function']($map);
    }

  }

  if ($use_client_auth) {
    //client side auth
    $session_query_map = _microstrategy_iframe_build_client_auth_parameters($config);
    $map = array_merge($map, $session_query_map);
  }
  else {
    //server side auth
    if (!empty($session_state)) {
      $map["usrSmgr"] = $session_state;
    }
  }

  //Process portlet 2 portlet message
  if (isset($config['id']) && !empty($config['id'])) {
    $name = MSTR_PREFIX . $config['id'];
    if (isset($_SESSION[$name])) {
      $msg_params = _microstrategy_url_string_to_parameter_table($_SESSION[$name]);
      $map = array_merge($map, $msg_params);
      _microstrategy_reset_content_url($map, $config);
    }
  }

  $src .= _microstrategy_parameter_table_to_url_string($map);

  return $src;
}

/**
 * create an array represents the task message
 * @param $config
 * @return unknown_type
 */
function _microstrategy_iframe_build_client_auth_parameters($config) {

  //default locale - current drupal locale
  $default_locale = _microstrategy_get_drupal_locale();
  $credentials = _microstrategy_get_mapped_credentials($config['sso_namespace'], $config['credential_mapper_file'], $config['credential_mapper_function']);

  $display_locale = _microstrategy_resolve_locale($credentials['display_locale'], $default_locale);
  $data_locale = _microstrategy_resolve_locale($credentials['data_locale'], $default_locale);
  $metadata_locale = _microstrategy_resolve_locale($credentials['metadata_locale'], $default_locale);
  $warehouse_data_locale = _microstrategy_resolve_locale($credentials['warehouse_data_locale'], $default_locale);

  $request = array(
    'server' => $config['iserver'],
    'port' => $config['port'],
    'project' => $config['project'],
    'connMode' => $config['authmode'],
    'dslid' => $display_locale,
    'nlid' => $data_locale,
    'mdlid' => $metadata_locale,
    'whlid' => $warehouse_data_locale,
    'rws' => $config['working_set_size'],
    'sf' => 0x00000010, //default session flag to remove read messages.
    'at' => '25', // set application type to Portal
    'ru' => 1,
  );

  return $request;

}

/**
 * Collect parameters that will be used in URL based on the type of user's selection.
 * The parameter name and value in the map is not encoded.
 * @param $config
 * @return array array of parameters
 */
function _microstrategy_collect_iframe_parameters($config) {

  $map = array();
  $map[MSTR_NAMESPACE_KEY] = MSTR_PREFIX . $config['id']; //used in server side

  //content url and content
  switch ($config['type']) {
    case MSTR_CNT_TYPE_REPORT:
      _microstrategy_iframe_add_url_segment($map, $config['report_url']);
      if ($config['report_name_type'] == MSTR_CNT_TYPE_IDENTIFIER_ID) {
        $map['reportID'] = $config['report_name'];
      }
      else {
        $map['reportName'] = $config['report_name'];
      }
      $map['reportViewMode'] = $config['report_view_mode'];
      $map['visMode'] = $config['report_visualization'];
      break;
    case MSTR_CNT_TYPE_PROMPT:
      _microstrategy_iframe_add_url_segment($map, $config['prompt_url']);
      if ($config['prompt_name_type'] == MSTR_CNT_TYPE_IDENTIFIER_ID) {
          $map['reportID'] = $config['prompt_name'];
      }
      else {
          $map['reportName'] = $config['prompt_name'];
      }
      break;
    case MSTR_CNT_TYPE_FOLDER:
      _microstrategy_iframe_add_url_segment($map, $config['folder_url']);
      if ($config['folder_name_type'] == MSTR_CNT_TYPE_IDENTIFIER_ID) {
        if (stripos($config['folder_name'], 'system:')) {
          $map['systemFolder'] = substr($config['folder_name'], 7);
        }
        else {
          $map["folderID"] = $config['folder_name'];
        }
      }
      else {
        $map["folderName"] = $config['folder_name'];
      }
      break;
    case MSTR_CNT_TYPE_HIST_LIST:
      _microstrategy_iframe_add_url_segment($map, $config['history_url']);
      break;
    case MSTR_CNT_TYPE_HTMLDOC:
      _microstrategy_iframe_add_url_segment($map, $config['htmldoc_url']);
      if ($config['htmldoc_name_type'] == MSTR_CNT_TYPE_IDENTIFIER_ID) {
          $map["documentID"] = $config['htmldoc_name'];
      }
      else {
          $map["documentName"] = $config['htmldoc_name'];
      }
      break;
    case MSTR_CNT_TYPE_DOC:
      _microstrategy_iframe_add_url_segment($map, $config['doc_url']);
      if ($config['doc_name_type'] == MSTR_CNT_TYPE_IDENTIFIER_ID) {
          $map["documentID"] = $config['doc_name'];
      }
      else {
          $map["documentName"] = $config['doc_name'];
      }
      if ($config['document_view_mode'] != 0) {
          $map["visMode"] = $config['document_view_mode'];
      }
      break;
    case MSTR_CNT_TYPE_PROJECT:
      _microstrategy_iframe_add_url_segment($map, $config['project_url']);
      break;
    case MSTR_CNT_TYPE_SEARCH:
      _microstrategy_iframe_add_url_segment($map, $config['search_url']);
      $map["folderType"] = "39";
      break;
  }

  //add shared parameters
  _microstrategy_add_shared_parameter($map, $config["shared_parameter"]);

  //add personal parameters
  _microstrategy_add_personal_parameter($map, $config["personal_parameter_template"], $config["personal_user_parameters"]);

  return $map;

}

/**
 * Resets the content URL part of the URL query string
 */
function _microstrategy_reset_content_url(&$map, $config) {

    switch ($map["evt"]) {
      case "4001":
        $url_string = $config['report_url'];
        break;
        break;
      case "2001":
        $url_string = $config['folder_url'];
        break;
      case "3018":
        $url_string = $config['history_url'];
        break;
      case "32001":
        $url_string = $config['htmldoc_url'];
        break;
      case "2048001":
      case "3135":
        $url_string = $config['doc_url'];
        break;
      case "3010":
        $url_string = $config['project_url'];
        break;
      case "3040":
        $url_string = $config['search_url'];
        break;
    }

    $new_params = array();//_microstrategy_parameter_str_to_table($url_string);
    _microstrategy_iframe_add_url_segment($new_params, $url_string);

    // deal with 3115 link. same as in mstrPortlet.getNewSrc.
    if (!isset($new_params["src"])) {
        $new_params["src"] = "";
    }

    // merge content url
    foreach ($new_params as $key => $value) {
      $map[$key] = $value;
    }
}

/**
 * Put a fragement into a parameter map.
 * @param &$map the parameter map to be updated.
 * @param $fragment a url fragement.
 *      It can take either encoded or not encoded form.
 *      For example, "a=xyz&b=123" or "a="x%20yz&b=123".
 */
function _microstrategy_iframe_add_url_segment(&$map, $fragment) {
  if (empty($fragment)) {
    return;
  }
  $parameters = explode("&", $fragment);
  foreach ($parameters as $param) {
    list($name, $val) = explode("=", $param . "="); //in case $param does not contain "="
    $name = microstrategy_decode_url($name);
    $val = microstrategy_decode_url($val);
    if (!empty($name)) {
      $map[$name] = $val;
    }
  }
}

/**
 * Add shared parameters to the parameter map.
 * @param array &$map the parameter map to be updated.
 * @param string $shared_parameter shared parameter.
 *      It can take either encoded or not encoded form. For example "name1:value1;name2:value2".
 */
function _microstrategy_add_shared_parameter(&$map, $shared_parameter) {
  if (empty($shared_parameter)) {
    return;
  }
  $parameters = explode(";", $shared_parameter);
  foreach ($parameters as $param) {
    list($name, $val) = explode(":", $param . ":"); //in case $param does not contain ":"
    $name = microstrategy_decode_url($name);
    $val = microstrategy_decode_url($val);
    if (!empty($name)) {
      $map[$name] = $val;
    }
  }
}

/**
 * Add personal parameters to the parameter map.
 * @param &$map the parameter map to be updated.
 * @param $template the personal parameter template.
 *      For example, "name1:display name1:default value1;name2:display name2:default value2".
 * @param $parameter personal parameters string.
 *      It can take either encoded or not encoded form. For example "name1:value1;name2:value2".
 */
function _microstrategy_add_personal_parameter(&$map, $template, $parameter) {
  if (empty($template)) return;
  module_load_include('inc', 'microstrategy', 'includes/microstrategy.parameter');
  $parameter_table = _microstrategy_get_personal_parameter_table($template, $parameter);
  // merge
  foreach ($parameter_table as $key => $value) {
    //value saved in map is not encoded
    $map[$key] = $value;
  }
}

/**
 * Convert a url string into a parameter table.
 * @param $parameter_str a parameter string. For example, "a=xyz&b=123".
 * @return the parameter table (of type array).
 */
function _microstrategy_url_string_to_parameter_table($url_str) {
  $map = array();
  if (empty($url_str)) {
    return $map;
  }
  $parameters = explode("&", $url_str);
  foreach ($parameters as $param) {
    list($name, $val) = explode("=", $param . "="); //in case $param does not contain "="
    if (!empty($name)) {
      $name = microstrategy_decode_url($name);
      $val = microstrategy_decode_url($val);
      $map[$name] = $val;
    }
  }
  return $map;
}

/**
 * Transfer a parameter map to a string presentation, which can be added to URL.
 * @param $map the parameter map
 * @return string the fragrament of url string presentation. Such as "a=xxx&b=xxx".
 */
function _microstrategy_parameter_table_to_url_string($map) {
  $param_str = "";
  $first = TRUE;
  foreach ($map as $key => $value) {
    //encode the parameter value in URL
    $name = microstrategy_encode_url($key);;//htmlentities($key, ENT_QUOTES, "UTF-8");//urlencode($key);
    if (isset($value)) {
      $val = microstrategy_encode_url($value);
      if (!$first) {
        $param_str .= "&";
      }
      $first = FALSE;
      $param_str .= $name . "=" . $val;
    }
  }
  return $param_str;
}

/**
 * Encode a message from an array for iframe
 * <iframe type="microstrategy.portlet.iframe" title="title" ...></iframe>
 */
function _microstrategy_encode_iframe_message($data) {
  $encoded_message = '<iframe';
  foreach ($data as $key => $value) {
    if (isset($value)) {
      $encoded_message .= ' ' . check_plain(trim($key)) .'="'. check_plain(trim($value)) . '"';
    }
  }
  return $encoded_message . '></iframe>';
}
