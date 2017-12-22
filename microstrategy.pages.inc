<?php

/**
 * @file
 * User page callbacks for the microstrategy module.
 */

/**
 * Menu callback which shows an preferences setting page for one MicroStrategy page.
 */
function microstrategy_page_preferences_form($form, &$form_state, $page) {

  drupal_set_title(t("%title preferences", array('%title' => $page->title)), PASS_THROUGH);

  $config = _microstrategy_get_page_merged_preferences($page->pid);
  $form['pid'] = array('#type' => 'value', '#value' => $page->pid);
  $form['page_path'] = array('#type' => 'value', '#value' => $page->path);

  //credential settings
  $show_logon = !_microstrategy_use_client_session($config['authmode']);
  $show_locale = $config['show_locale'];
  $sso_namespace = $config['sso_namespace'];
  $form['show_logon'] = array('#type' => 'value', '#value' => $show_logon);
  $form['show_locale'] = array('#type' => 'value', '#value' => $show_locale);
  $form['sso_namespace'] = array('#type' => 'value', '#value' => $sso_namespace);

  if ( $show_logon || $show_locale) {
    $form['credential'] = _microstrategy_credential_fieldset($show_logon, $show_locale, $sso_namespace);
  }

  $form['content'] = _microstrategy_content_fieldset($config);
  $form['display'] = _microstrategy_display_fieldset($config);

  //personal_user_parameters
  if (!empty($config["personal_parameter_template"])) {
    $form['parameter'] = _microstrategy_personal_parameter_fieldset($config);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

/**
 * Menu callback which shows an preferences setting page for one MicroStrategy block.
 */
function microstrategy_block_preferences_form($form, &$form_state, $bid) {

  drupal_set_title(t("%title preferences", array('%title' => _microstrategy_get_block_title($bid))), PASS_THROUGH);

  $config = _microstrategy_get_block_merged_preferences($bid);
  $block_type = _microstrategy_get_block_type($bid);

  $form = array();

  $form['bid'] = array('#type' => 'value', '#value' => $bid);
  $form['block_type'] = array('#type' => 'value', '#value' => $block_type);

  //credential settings
  $show_logon = !_microstrategy_use_client_session($config['authmode']);
  $show_locale = $config['show_locale'];
  $sso_namespace = $config['sso_namespace'];
  $form['show_logon'] = array('#type' => 'value', '#value' => $show_logon);
  $form['show_locale'] = array('#type' => 'value', '#value' => $show_locale);
  $form['sso_namespace'] = array('#type' => 'value', '#value' => $sso_namespace);
  if ( $show_logon || $show_locale) {
    $form['credential'] = _microstrategy_credential_fieldset($show_logon, $show_locale, $sso_namespace);
  }

  if ($block_type == MSTR_TYPE_BASIC) {
    $form['content'] = _microstrategy_content_fieldset($config, $bid);
    $form['display'] = _microstrategy_display_fieldset($config);
  }
  elseif ($block_type == MSTR_TYPE_MASTER) {
    $form['content'] = _microstrategy_master_content_fieldset($config, $bid);
    $form['display'] = _microstrategy_display_fieldset($config);
    $form['master'] = _microstrategy_master_properties_fieldset($config);
  }

  //personal_user_parameters
  if (!empty($config["personal_parameter_template"])) {
    $form['parameter'] = _microstrategy_personal_parameter_fieldset($config);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#prefix' => '<div class="container-inline">',
    '#weight' => 2,
  );

  $path = isset($_REQUEST['destination'])? urlencode($_REQUEST['destination']) : '';
  $cancel = l(t('Cancel'), $path);
  $form['cancel'] = array(
    '#value' => $cancel,
    '#suffix' => '</div>',
    '#weight' => 3,
  );

  $form['#attributes'] = array('class' => 'confirmation');

  return $form;

}

function _microstrategy_credential_fieldset($show_logon, $show_locale, $sso_namespace) {

  global $user;

  $credential = array(
    '#type' => 'fieldset',
    '#title' => t('Enter credential information to connect to MicroStrategy:'),
    '#collapsible' => TRUE,
    '#description' => t('All MicroStrategy pages and blocks with the same Single Sign-on Namespace share the same MicroStrategy Credentials for this user.'),
  );

  $mstr_cred = _microstrategy_get_credentials($sso_namespace);

  if ($show_logon) {
    $credential['logon'] = array(
      '#type' => 'textfield',
      '#title' => t('Logon'),
      '#default_value' => isset($mstr_cred['logon']) ? $mstr_cred['logon'] : '',
      '#size' => 25,
      '#required' => TRUE,
    );
    $credential['password'] = array(
        '#type' => 'password',
        '#title' => t('Password'),
        '#size' => 25,
        '#required' => TRUE,
    );
  }

  if ($show_locale) {
    $options = array(
      MSTR_PORTLET_LOCALE_DEFAULT => t('Default'),
      MSTR_PORTLET_LOCALE_GERMAN => t('German'),
      MSTR_PORTLET_LOCALE_SPANISH => t('Spanish'),
      MSTR_PORTLET_LOCALE_ENGLISH => t('English'),
      MSTR_PORTLET_LOCALE_ENGLISH_UK => t('English (UK)'),
      MSTR_PORTLET_LOCALE_FRENCH => t('French'),
      MSTR_PORTLET_LOCALE_ITALIAN => t('Italian'),
      MSTR_PORTLET_LOCALE_JAPANESE => t('Japanese'),
      MSTR_PORTLET_LOCALE_KOREAN => t('Korean'),
      MSTR_PORTLET_LOCALE_PORTUGUESE_BR => t('Portuguese (Brazil)'),
      MSTR_PORTLET_LOCALE_SWEDISH => t('Swedish'),
      MSTR_PORTLET_LOCALE_CHINESE_S => t('Chinese (Simplified)'),
      MSTR_PORTLET_LOCALE_CHINESE_T => t('Chinese (Traditional)'),
    );
    $credential['display_locale'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#default_value' => isset($mstr_cred['display_locale']) ? $mstr_cred['display_locale'] : MSTR_PORTLET_LOCALE_DEFAULT,
      '#options' => $options,
    );
    $credential['data_locale'] = array(
        '#type' => 'select',
        '#title' => t('Number and Date Format'),
        '#default_value' => isset($mstr_cred['data_locale']) ? $mstr_cred['data_locale'] : MSTR_PORTLET_LOCALE_DEFAULT,
        '#options' => $options,
    );
    $credential['metadata_locale'] = array(
        '#type' => 'select',
        '#title' => t('Metadata'),
        '#default_value' => isset($mstr_cred['metadata_locale']) ? $mstr_cred['metadata_locale'] : MSTR_PORTLET_LOCALE_DEFAULT,
        '#options' => $options,
    );
    $credential['warehouse_data_locale'] = array(
        '#type' => 'select',
        '#title' => t('Data'),
        '#default_value' => isset($mstr_cred['warehouse_data_locale']) ? $mstr_cred['warehouse_data_locale'] : MSTR_PORTLET_LOCALE_DEFAULT,
        '#options' => $options,
    );
  }

  return $credential;
}

function _microstrategy_content_fieldset($config, $bid=0) {
  $content = array(
    '#type' => 'fieldset',
    '#title' => t('Select MicroStrategy business intelligence content to display:'),
    '#collapsible' => TRUE,
    '#theme' => 'microstrategy_content',
  );

  //for building "load last session setting" button
  $content['bid'] = array('#type' => 'value', '#value' => $bid);

  $content['type'] = array(
    '#type' => 'radios',
    '#default_value' => $config['type'],
    '#options' => array(
      MSTR_CNT_TYPE_REPORT => t('Display MicroStrategy Report'),
      MSTR_CNT_TYPE_FOLDER => t('Display MicroStrategy Folder'),
      MSTR_CNT_TYPE_HIST_LIST => t('Display MicroStrategy History List'),
      MSTR_CNT_TYPE_SEARCH => t('Display MicroStrategy Search Page'),
      MSTR_CNT_TYPE_PROJECT => t('Display MicroStrategy Project Page'),
      MSTR_CNT_TYPE_HTMLDOC => t('Display MicroStrategy HTML Document'),
      MSTR_CNT_TYPE_DOC => t('Display MicroStrategy Document')),
  );

  $content['report_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['report_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['report_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['report_name'],
  );
  $content['report_view_mode'] = array(
    '#type' => 'select',
    '#default_value' => $config['report_view_mode'],
    '#options' => array(
      1 => t('Grid'),
      2 => t('Graph'),
      3 => t('Grid and Graph')),
  );
  $content['report_visualization'] = array(
    '#type' => 'select',
    '#default_value' => $config['report_visualization'],
    '#options' => array(
      0 => t('None'),
      50 => t('Flash Visualization'),
      51 => t('AJAX Visualization')),
  );

  $content['folder_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['folder_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['folder_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['folder_name'],
  );

  $content['htmldoc_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['htmldoc_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['htmldoc_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['htmldoc_name'],
  );

  $content['doc_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['doc_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['doc_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['doc_name'],
  );
  $content['doc_view_mode'] = array(
    '#type' => 'select',
    '#default_value' => $config['document_view_mode'],
    '#options' => array(
      0 => t('None'),
      50 => t('Flash Visualization'),
      51 => t('AJAX Visualization')),
  );

  return $content;
}

function _microstrategy_master_content_fieldset($config, $bid=0) {
  $content = array(
    '#type' => 'fieldset',
    '#title' => t('Select MicroStrategy business intelligence content to display:'),
    '#collapsible' => TRUE,
    '#theme' => 'microstrategy_master_content',
  );

  //for building "load last session setting" button
  $content['bid'] = array('#type' => 'value', '#value' => $bid);

  $content['type'] = array(
    '#type' => 'radios',
    '#default_value' => $config['type'],
    '#options' => array(
      MSTR_CNT_TYPE_PROMPT => t('Display MicroStrategy Prompts (using a prompted report)'),
      MSTR_CNT_TYPE_FOLDER => t('Display MicroStrategy Folder'),
      MSTR_CNT_TYPE_HIST_LIST => t('Display MicroStrategy History List'),
      MSTR_CNT_TYPE_SEARCH => t('Display MicroStrategy Search Page'),
      MSTR_CNT_TYPE_REPORT => t('Display MicroStrategy Report'),
      MSTR_CNT_TYPE_DOC => t('Display MicroStrategy Document'),
    ),
  );

  $content['prompt_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['prompt_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['prompt_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['prompt_name'],
  );

  $content['folder_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['folder_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['folder_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['folder_name'],
  );

  $content['report_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['report_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['report_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['report_name'],
  );
  $content['report_view_mode'] = array(
    '#type' => 'select',
    '#default_value' => $config['report_view_mode'],
    '#options' => array(
      1 => t('Grid'),
      2 => t('Graph'),
      3 => t('Grid and Graph')),
  );
  $content['report_visualization'] = array(
    '#type' => 'select',
    '#default_value' => $config['report_visualization'],
    '#options' => array(
      0 => t('None'),
      50 => t('Flash Visualization'),
      51 => t('AJAX Visualization')),
  );

  $content['doc_name_type'] = array(
    '#type' => 'select',
    '#default_value' => $config['doc_name_type'],
    '#options' => array(
      MSTR_CNT_TYPE_IDENTIFIER_ID => t('ID'),
      MSTR_CNT_TYPE_IDENTIFIER_NAME => t('Name')),
  );
  $content['doc_name'] = array(
    '#type' => 'textfield',
    '#size' => 45,
    '#default_value' => $config['doc_name'],
  );
  $content['doc_view_mode'] = array(
    '#type' => 'select',
    '#default_value' => $config['document_view_mode'],
    '#options' => array(
      0 => t('None'),
      50 => t('Flash Visualization'),
      51 => t('AJAX Visualization')),
  );

  return $content;
}

/**
 * Theme microstrategy content selector form.
 *
 * @ingroup themeable
 */
function theme_microstrategy_content($variables) {
  $form = $variables['form'];
  $rows = array(
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_REPORT]),
      drupal_render($form['report_name_type']),
      drupal_render($form['report_name']),
      drupal_render($form['report_view_mode']),
      drupal_render($form['report_visualization']),
    ),
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_FOLDER]),
      drupal_render($form['folder_name_type']),
      drupal_render($form['folder_name']),
    ),
    array( drupal_render($form['type'][MSTR_CNT_TYPE_HIST_LIST]) ),
    array( drupal_render($form['type'][MSTR_CNT_TYPE_SEARCH]) ),
    array( drupal_render($form['type'][MSTR_CNT_TYPE_PROJECT]) ),
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_HTMLDOC]),
      drupal_render($form['htmldoc_name_type']),
      drupal_render($form['htmldoc_name']),
    ),
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_DOC]),
      drupal_render($form['doc_name_type']),
      drupal_render($form['doc_name']),
      drupal_render($form['doc_view_mode']),
    ) );

  $attributes = array(
    'width' => '100%',
  );

  $output = theme('table', array('rows' => $rows, 'attributes' => $attributes));

  // build 'Load last session settings' button.
  $bid = $form['bid']['#value'];
  if ($bid > 0) {
    _microstrategy_move_message_from_cookie_to_session($bid);
    $name = MSTR_PREFIX . $bid;
    if (isset($_SESSION[$name]) && !empty($_SESSION[$name])) {
      $onclick = "mstrPortletEdit.message='" . check_plain($_SESSION[$name])
          . "';mstrPortletEdit.loadSettingsFromMessage(this.form);this.disabled=true;";
      $output .= '<input type="button" onclick="' . $onclick . '" value="'
          . t('Load last session settings') . '"</>';
      _microstrategy_block_preference_load_js();
    }
  }

  return $output;
}

/**
 * Theme microstrategy master content selector form.
 *
 * @ingroup themeable
 */
function theme_microstrategy_master_content($variables) {
  $form = $variables['form'];

  _microstrategy_master_block_preference_load_js();

  $rows = array(
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_PROMPT]),
      drupal_render($form['prompt_name_type']),
      drupal_render($form['prompt_name']),
    ),
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_FOLDER]),
      drupal_render($form['folder_name_type']),
      drupal_render($form['folder_name']),
    ),
    array( drupal_render($form['type'][MSTR_CNT_TYPE_HIST_LIST]) ),
    array( drupal_render($form['type'][MSTR_CNT_TYPE_SEARCH]) ),
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_REPORT]),
      drupal_render($form['report_name_type']),
      drupal_render($form['report_name']),
      drupal_render($form['report_view_mode']),
      drupal_render($form['report_visualization']),
    ),
    array(
      drupal_render($form['type'][MSTR_CNT_TYPE_DOC]),
      drupal_render($form['doc_name_type']),
      drupal_render($form['doc_name']),
      drupal_render($form['doc_view_mode']),
    ),
  );

  $attributes = array(
    'width' => '100%',
  );

  $output = theme('table', array('rows' => $rows, 'attributes' => $attributes));

  // build 'Load last session settings' button.
  $bid = $form['bid']['#value'];
  if ($bid > 0) {
    _microstrategy_move_message_from_cookie_to_session($bid);
    $name = MSTR_PREFIX . $bid;
    if (isset($_SESSION[$name]) && !empty($_SESSION[$name])) {
      $onclick = "mstrPortletMasterEdit.message='" . check_plain($_SESSION[$name])
          . "';mstrPortletMasterEdit.loadSettingsFromMessage(this.form);this.disabled=true;";
      $output .= '<input type="button" onclick="' . $onclick . '" value="'
          . t('Load last session settings') . '"</>';
    }
  }

  return $output;
}

/**
 * Build properties fieldset for a microstrategy master block.
 * @param $config Block configurations.
 * @param $build_portlet_options Build drop down list options from cookie or not.
 */
function _microstrategy_master_properties_fieldset($config, $build_portlet_options = TRUE) {
  $output = array(
    '#type' => 'fieldset',
    '#title' => t('Set master block properties'),
    '#collapsible' => TRUE,
  );
  $output['show_sendto'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display Send-to Menu'),
    '#default_value' => $config['show_sendto'],
  );

  $default_target = isset($config['default_target']) ? $config['default_target'] : NULL;
  //build options.
  $options = array();
  if (!is_null($default_target)) {
    $options[$default_target] = $default_target;
  }

  $output['default_target'] = array(
    '#type' => 'select',
    '#title' => t('Default Target Block'),
    '#default_value' => $default_target,
    '#options' => $options,
    '#attributes'=> array(
        'onfocus' => 'mstrPortletMasterEdit.buildTargetPortletDropDown();',
        'project' => $config['project'],
        'tportlet' => $default_target),
    	'#validated' => TRUE, //TQMS 461716 skip validation
  );

  $output['enable_all_for_send_as_filter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable All For Send as Filter'),
    '#default_value' => $config['enable_all_for_send_as_filter'],
  );

  return $output;
}

function _microstrategy_display_fieldset($config) {
  $output = array(
    '#type' => 'fieldset',
    '#title' => t('Choose display properties:'),
    '#collapsible' => TRUE,
  );
  $output['height'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#size' => 25,
    '#default_value' => $config['height'],
  );
  $output['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#size' => 25,
    '#default_value' => $config['width'],
  );
  $output['scrolling'] = array(
    '#type' => 'select',
    '#title' => t('Scrolling'),
    '#default_value' => $config['scrolling'],
    '#options' => array(t('Auto'), t('Yes'), t('No')),
  );

  return $output;
}

function _microstrategy_personal_parameter_fieldset($config) {

  $template = $config["personal_parameter_template"];
  if (empty($template)) {
    return "";
  }

  $output = array(
    '#type' => 'fieldset',
    '#title' => t('User Parameters'),
    '#collapsible' => TRUE,
  );

  $output['template'] = array('#type' => 'value', '#value' => $template);

  $parameter = $config["personal_user_parameters"];

  module_load_include('inc', 'microstrategy', 'includes/microstrategy.parameter');
  $template_table = _microstrategy_string_to_template_table($template);
  $parameter_table = _microstrategy_get_personal_parameter_table($template, $parameter);

  foreach ($parameter_table as $name => $value) {
    $output['MSTRDP_' . check_plain($name)] = array(
      '#type' => 'textfield',
      '#title' => check_plain($template_table[$name]["display"]),
      '#size' => 45,
      '#default_value' => $value,
    );
  }

  return $output;
}

function microstrategy_page_preferences_form_validate($form, &$form_state) {
  _microstrategy_validate_content_settings($form_state);
}

function microstrategy_block_preferences_form_validate($form, &$form_state) {
  _microstrategy_validate_content_settings($form_state);
}

function microstrategy_page_preferences_form_submit($form, &$form_state) {
  _microstrategy_save_credential($form_state['values']);
  _microstrategy_save_page_preferences($form_state['values']);

  watchdog('microstrategy', '%title preferences have been saved.',
    array('%title' => _microstrategy_get_page_title($form_state['values']['pid'])), WATCHDOG_NOTICE);

  $form_state['redirect'] = $form_state['values']['page_path'];
}

function microstrategy_block_preferences_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Submit')) {
    _microstrategy_save_credential($form_state['values']);
    _microstrategy_save_block_preferences($form_state['values']);
    _microstrategy_remove_message_from_session($form_state['values']['bid']);

    watchdog('microstrategy', '%title preferences have been saved.',
        array('%title' => _microstrategy_get_block_title($form_state['values']['bid'])), WATCHDOG_NOTICE);
  }
  elseif ($form_state['values']['op'] == t('Cancel')) {
    //do nothing
  }
}

function _microstrategy_save_page_preferences($values) {
  _microstrategy_save_preferences($values, $values['pid'], 0, MSTR_TYPE_BASIC);
}

function _microstrategy_save_block_preferences($values) {
  _microstrategy_save_preferences($values, 0, $values['bid'], $values['block_type']);
}

function _microstrategy_save_credential($values) {

  $show_logon = isset($values['show_logon']) ? $values['show_logon'] : FALSE;
  $show_locale = isset($values['show_locale']) ? $values['show_locale'] : FALSE;
  $sso_namespace = isset($values['sso_namespace']) ? $values['sso_namespace'] : '';

  if ((!$show_logon && !$show_locale)) {
    return;
  }

  global $user;
  $changed = FALSE;

  $old_credentials = _microstrategy_get_credentials($sso_namespace);

  $new_credentials = array();

  if ($show_logon) {
    module_load_include('inc', 'microstrategy', 'includes/microstrategy.crypto');
    $crypto = new MicrostrategyCrypto();
    $new_credentials['logon'] = trim($values['logon']);
    $new_credentials['password'] = $values['password'];
    if ($new_credentials['logon'] != $old_credentials['logon']
        || $new_credentials['password'] != $old_credentials['password']) {
      $changed = TRUE;
    }
  }

  if ($show_locale) {
    $new_credentials['display_locale'] = $values['display_locale'];
    $new_credentials['data_locale'] = $values['data_locale'];
    $new_credentials['metadata_locale'] = $values['metadata_locale'];
    $new_credentials['warehouse_data_locale'] = $values['warehouse_data_locale'];
    if ($new_credentials['display_locale'] != $old_credentials['display_locale']
        || $new_credentials['data_locale'] != $old_credentials['data_locale']
        || $new_credentials['metadata_locale'] != $old_credentials['metadata_locale']
        || $new_credentials['warehouse_data_locale'] != $old_credentials['warehouse_data_locale']) {
      $changed = TRUE;
    }
  }

  _microstrategy_save_credentials($sso_namespace, $new_credentials);

  if ($changed) {
    //TQMS:238068 close session if user tries to save credentials
    module_load_include('inc', 'microstrategy', 'includes/microstrategy.session');
    microstrategy_close_all_sessions();
  }

}

function _microstrategy_save_preferences($values, $pid, $bid, $block_type = MSTR_TYPE_BASIC) {
  global $user;
  $config =  new stdClass();
  $config->uid = $user->uid;
  $config->pid = $pid;
  $config->bid = $bid;

  $config->type = $values['type'];
  $config->folder_name_type = $values['folder_name_type'];
  $config->folder_name = $values['folder_name'];
  $config->report_name_type = $values['report_name_type'];
  $config->report_name = $values['report_name'];
  $config->report_view_mode = $values['report_view_mode'];
  $config->report_visualization = $values['report_visualization'];
  $config->doc_name_type = $values['doc_name_type'];
  $config->doc_name = $values['doc_name'];
  $config->document_view_mode = $values['doc_view_mode'];

  if ($block_type == MSTR_TYPE_BASIC) {
    $config->htmldoc_name_type = $values['htmldoc_name_type'];
    $config->htmldoc_name = $values['htmldoc_name'];
  }
  elseif ($block_type == MSTR_TYPE_MASTER) {
    $config->prompt_name_type = $values['prompt_name_type'];
    $config->prompt_name = $values['prompt_name'];
    $config->show_sendto = $values['show_sendto'];
    $config->default_target = $values['default_target'];
    $config->enable_all_for_send_as_filter = $values['enable_all_for_send_as_filter'];
  }

  $config->height = $values['height'];
  $config->width = $values['width'];
  $config->scrolling = $values['scrolling'];

  //personal_user_parameters
  if (isset($values['template']) && !empty($values['template'])) {
    $template = $values['template'];
    module_load_include('inc', 'microstrategy', 'includes/microstrategy.parameter');
    $default_parameter_table = _microstrategy_get_personal_parameter_table($template, "");
    $new_parameter_table = array();
    foreach ($default_parameter_table as $name => $value) {
      $new_parameter_table[$name] = $values['MSTRDP_' . $name];
    }
    $config->personal_user_parameters = _microstrategy_parameter_table_to_string($new_parameter_table);
  }

  // Remove any old preferences
  db_delete('microstrategy_preferences')
    ->condition('uid', $config->uid)
    ->condition('pid', $config->pid)
    ->condition('bid', $config->bid)
    ->execute();

  // Update preferences
  drupal_write_record('microstrategy_preferences', $config);

}

function _microstrategy_validate_content_settings($form_state) {
  //validation
  $name = "";
  $error = "";
  $cnt_type = $form_state['values']['type'];
  switch ($cnt_type) {
    case MSTR_CNT_TYPE_REPORT:
      $name = 'report_name';
      if (empty($form_state['values'][$name])) {
        $error = t('Report Name or ID cannot be empty!');
      }
      break;
    case MSTR_CNT_TYPE_PROMPT:
      $name = 'prompt_name';
      if (empty($form_state['values'][$name])) {
        $error = t('Report Name or ID cannot be empty!');
      }
      break;
    case MSTR_CNT_TYPE_FOLDER:
      $name = 'folder_name';
      if (empty($form_state['values'][$name])) {
        $error = t('Folder Name or ID cannot be empty!');
      }
      break;
    case MSTR_CNT_TYPE_HTMLDOC:
      $name = 'htmldoc_name';
      if (empty($form_state['values'][$name])) {
        $error = t('HTML Document Name or ID cannot be empty!');
      }
      break;
    case MSTR_CNT_TYPE_DOC:
      $name = 'doc_name';
      if (empty($form_state['values'][$name])) {
        $error = t('Document Name or ID cannot be empty!');
      }
      break;
  }

  if (!empty($name) && !empty($error)) {
    form_set_error($name, $error);
  }

}
