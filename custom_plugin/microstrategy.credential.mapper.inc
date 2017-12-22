<?php

/**
 * @file
 * This custom credential mapper sample code demonstrates how to implement
 * a use case for Single Sign-On.
 * In this use case, the Drupal user is not required to input any
 * MicroStrategy credentials, instead this class can automatically
 * map the MicroStrategy credentials for the Drupal user.
 * The Drupal administrator need to set the authentication mode
 * to anonymous to remove the input fields for credentials, and
 * use the full qualified name of this file as "Credential Mapper
 * File Name".
 */

/**
 *
 * Mapping the credentials to a new set of credentials, which can be the same.
 *
 * The Drupal administrator needs to use the full qualified name of
 * this file as "Credential Mapper File Name" and the full
 * qualified name of this function as "Credential Mapper Function Name".
 *
 * @param $credentials the credentials (of type array) including elements
 *      $credentials['logon'],
 *      $credentials['password'],
 *      $credentials['display_locale'],
 *      $credentials['data_locale'],
 *      $credentials['metadata_locale'],
 *      $credentials['warehouse_data_locale'].
 * @return the new credentials (of type array).
 */
function microstrategy_map_credentials($credentials) {
  
  //To get drupal's users identity.
  //global $user;
  
  //To map all users to a same microstrategy user.
  //$credentials['logon'] = 'demo';
  //$credentials['password'] = 'demo';
  
  //The following code postfixes "_MSTR" to the login passed in $credentials. 
  //You should implement the logic to get the real credentials. 
  //You can use whatever information available to do the mapping.
  if (isset($credentials['logon'])) {
    $uid = $credentials['logon'] . "_MSTR";
    $credentials['logon'] = $uid;
  }
  
  return $credentials;
  
}