<?php

/**
* @file
* MicroStrategy personal parameters builder file.
*/

/**
 * Return the personal parameters table.
 * @param $template The personal parameter template string.
 * @param $parameters The personal parameter string.
 * @return The personal parameter table (of type array).
 */
function _microstrategy_get_personal_parameter_table($template, $parameters) {
  $template_table = _microstrategy_string_to_template_table($template);
  $parameter_table = _microstrategy_string_to_parameter_table($parameters);
  return _microstrategy_get_updated_parameter_table($template_table, $parameter_table);
}

/**
 * Convert personal template string to a map.
 * @param $template_str a personal parameter template string. For example, "a=xyz&b=123".
 * @return A template table (type of array).
 */
function _microstrategy_string_to_template_table($template_str) {
  $map = array();
  if (empty($template_str)) {
    return $map;
  }
  $parameters = explode(";", $template_str);
  foreach ($parameters as $param) {
    list($name, $display, $default) = explode(":", $param . "::");
    if (!empty($name)) {
      $name = microstrategy_decode_url($name);
      if (!empty($display)) {
        $display = microstrategy_decode_url($display);
      }
      if (!empty($default)) {
        $default = microstrategy_decode_url($default);
      }
      if (!empty($name)) {
        $map[$name] = array("display" => $display, "default" => $default);
      }
    }
  }
  return $map;
}

/**
 * Return personal parameter table combined with template table.
 * Any parameter is not defined in template will be ignored.
 * Any parameter is defined in template table but not in parameter table will be added to parameter table with default value.
 * Any parameter is defined in parameter table and template table will remain the same.
 * @param $template_table the template table
 * @param $parameter_table the parameter table
 * @return new parameter table
 */
function _microstrategy_get_updated_parameter_table($template_table, $parameter_table) {
  $new_param_table = array();
  foreach ($template_table as $name => $value) {
    if (isset($parameter_table[$name]) && !empty($parameter_table[$name])) {
      $new_param_table[$name] = $parameter_table[$name];
    }
    else {
      $new_param_table[$name] = $value['default'];
    }
  }
  return $new_param_table;
}

/**
 * Convert a parameter string into a parameter table.
 * @param $parameter_str a parameter string. For example, "a:xyz;b:123".
 * @return the parameter table (of type array). 
 */
function _microstrategy_string_to_parameter_table($parameter_str) {
  $map = array();
  if (empty($parameter_str)) {
    return $map;
  }
  $parameters = explode(";", $parameter_str);
  foreach ($parameters as $param) {
    list($name, $val) = explode(":", $param . ":"); //in case $param does not contain "="
    if (!empty($name)) {
      $name = microstrategy_decode_url($name);
      $val = microstrategy_decode_url($val);
      $map[$name] = $val;
    }
  }
  return $map;
}

/**
 * Transfer a parameter map to a string presentation.
 * @param $map the parameter map
 * @return string the fragrament of string presentation. Such as "a:xxx;b:xxx".
 */
function _microstrategy_parameter_table_to_string($map) {
  $param_str = "";
  $first = TRUE;
  foreach ($map as $key => $value) {
    //encode the parameter value in URL
    $name = microstrategy_encode_url($key);
    if (isset($value)) {
      $val = microstrategy_encode_url($value);
      if (!$first) {
        $param_str .= ";";
      }
      $first = FALSE;
      $param_str .= $name . ":" . $val;
    }
  }
  return $param_str;
}
