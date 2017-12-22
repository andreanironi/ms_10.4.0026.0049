<?php

/**
 * This custom URL Parameter mapper demonstrates how to attach
 * personalized parameter to the MicroStrategy URL.
 * In this use case, this class will lookup the storeID of the
 * Drupal server and append the storeID to the URL. The Drupal
 * user doesn't need to his storeID in the portlet.
 *
 * The Customer who use this scenario should customize MicroStrategy
 * Web to handle the new custom parameter.
 * The Drupal administrator needs to use the full qualified name of
 * this file as "URL Parameter Mapper File Name" and the full
 * qualified name of this function as "URL Parameter Mapper Function Name".
 * 
 * @param $parameters the url parameters (of type array).
 * @return the new url parameters (of type array).
 */
function microstrategy_map_url_parameters($parameters) {
  
  //get store ID
  $store_id = _microstrategy_get_store_id();
  //put store ID in the url parameter array
  $parameters["storeID"] = $store_id;
  
  return $parameters;
}

/** To be finished by customer
 * @return the store ID
 */
function _microstrategy_get_store_id() {
  
  //To get drupal's users identity.
  //global $user;

  // Customer should provide the implementation of this method,
  //the store ID can be retrievaled from any external identity storage,
  //such as LDAP, or Drupal provided storage.
  return "ABC";
}