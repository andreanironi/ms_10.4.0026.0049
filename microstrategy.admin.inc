<?php

/**
 * @file
 * MicroStrategy admin file, takes care admin interface for MicroStrategy
 */

/**
 * Menu callback which shows an overview page.
 */
function microstrategy_overview_page() {
  $content = array();

  $page['title'] = t('Pages');
  $page['href'] = 'admin/structure/microstrategy/page';
  $page['localized_options'] = array();
  $page['description'] = t('List, edit, or add MicroStrategy pages. A MicroStrategy page is a Drupal page that displays MicroStrategy content (such as reports, documents, folders, and so on).');
  $content[] = $page;

  $block['title'] = t('Blocks');
  $block['href'] = 'admin/structure/microstrategy/block';
  $block['localized_options'] = array();
  $block['description'] = t('List, edit, or add MicroStrategy blocks. A MicroStrategy block is a Drupal block that displays MicroStrategy content (such as reports, documents, folders, and so on).');
  $content[] = $block;

  $page['title'] = t('Default Settings');
  $page['href'] = 'admin/structure/microstrategy/defaultsettings';
  $page['localized_options'] = array();
  $page['description'] = t('Set the default settings for MicroStrategy pages or blocks. The changes are applied to any newly added and non-customized existing pages or blocks.');
  $content[] = $page;

  return theme('admin_block_content', array('content' => $content));
}

/**
 * Menu callback which shows an overview page.
 */
function microstrategy_default_settings_page() {
  $content = array();

  $page['title'] = t('Page Default Settings');
  $page['href'] = 'admin/structure/microstrategy/page/' . MSTR_PAGE_DEFA_SETT_ID . '/settings';
  $page['localized_options'] = array();
  $page['description'] = t('Set the default settings for MicroStrategy pages. The changes are applied to any newly added pages, as well as non-customized existing pages.');
  $content[] = $page;

  $page['title'] = t('Basic block default settings');
  $page['href'] = 'admin/structure/microstrategy/block/' . MSTR_BASIC_BLOCK_DEFA_SETT_ID . '/settings';
  $page['localized_options'] = array();
  $page['description'] = t('Set the default settings for MicroStrategy basic blocks. The changes are applied to any newly added basic blocks, as well as non-customized existing basic blocks.');
  $content[] = $page;

  $page['title'] = t('Master block default settings');
  $page['href'] = 'admin/structure/microstrategy/block/' . MSTR_MASTER_BLOCK_DEFA_SETT_ID . '/settings';
  $page['localized_options'] = array();
  $page['description'] = t('Set the default settings for MicroStrategy master blocks. The changes are applied to any newly added master blocks, as well as non-customized existing master blocks.');
  $content[] = $page;

  return theme('admin_block_content', array('content' => $content));
}

function microstrategy_page_admin_form($form, &$form_state) {
  $form['#theme'] = 'microstrategy_page_admin_form';
  return $form;
}

/**
 * Theme the new-page form.
 *
 * @ingroup themeable
 */
function theme_microstrategy_page_admin_form($variables) {
  $form = $variables['form'];
  $pages = microstrategy_get_pages();

  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => 3));
  $rows = NULL;
  if (is_array($pages) && !empty($pages)) {
    foreach ($pages as $page) {
      $path = empty($page->path) ? 'microstrategy/page/' . $page->pid : $page->path;
      $title = l($page->title, $path);
      $edit = l(t('edit'), 'admin/structure/microstrategy/page/'. $page->pid . '/edit');
      $settings = l(t('settings'), 'admin/structure/microstrategy/page/'. $page->pid . '/settings');
      $delete = l(t('delete'), 'admin/structure/microstrategy/page/'. $page->pid . '/delete');
      $rows[] = array($title, $edit, $settings, $delete);
    }
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows));

  return $output;
}

function microstrategy_page_edit_form($form, &$form_state, $pid = 0) {
  $page = NULL;
  if ($pid > 0) {
    $page = db_query('SELECT * FROM {microstrategy_pages} WHERE pid = :pid', array(':pid' => $pid))->fetch();
  }

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Page Title'),
    '#default_value' => $page ? $page->title : '',
    '#size' => 32,
    '#required' => TRUE,
    '#maxlength' => 64,
    '#description' => t('The title for this page.'),
  );
  // Path alias
  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#default_value' => $page ? $page->path : '',
    '#size' => 32,
    '#required' => TRUE,
    '#maxlength' => 255,
    '#description' => t('Specify a URL by which this page can be accessed. Use a relative path and do not use trailing slashes.'),
  );
  $form['menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create a menu entry'),
    '#description' => t('If selected, a menu entry will be created for this page.'),
    '#default_value' => $page ? $page->menu : 1,
  );

  // Role-based visibility settings
  $default_role_options = array();
  $result = db_query("SELECT rid FROM {microstrategy_pages_roles} WHERE pid = :pid", array(':pid' => $pid));
  foreach ($result as $role) {
    $default_role_options[] = $role->rid;
  }
  $result = db_query('SELECT rid, name FROM {role} ORDER BY name');
  $role_options = array();
  foreach ($result as $role) {
    $role_options[$role->rid] = $role->name;
  }
  $form['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Show page for specific roles'),
    '#default_value' => $default_role_options,
    '#options' => $role_options,
    '#description' => t('Show this page only for the selected role(s).'),
  );

  if ($page) {
    // edit page
    $form['pid'] = array(
      '#type' => 'value',
      '#value' => $pid,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save page'),
      '#weight' => 2,
    );
  }
  else {
    //add page
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add page'),
      '#weight' => 2,
    );
  }

  return $form;
}

function microstrategy_page_edit_form_validate($form, &$form_state) {
  if ($form_state['values']['op'] == t('Add page') || $form_state['values']['op'] == t('Save page')) {
    $pid = isset($form_state['values']['pid']) ? $form_state['values']['pid'] : NULL;
    _microstrategy_check_path($form_state['values']['path'], $pid);
  }
}

function microstrategy_page_edit_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Add page')) {
    //title, path, menu
    $pid = db_insert('microstrategy_pages')
      ->fields(array(
        'title' => $form_state['values']['name'],
        'path' => $form_state['values']['path'],
        'menu' => $form_state['values']['menu'],
      ))
      ->execute();
    //roles
    foreach (array_filter($form_state['values']['roles']) as $rid) {
      db_insert('microstrategy_pages_roles')
        ->fields(array(
          'pid' => $pid,
          'rid' => $rid,
        ))
        ->execute();
    }
    //log
    $t_args = array('%title' => $form_state['values']['name']);
    drupal_set_message(t('Page %title has been added.', $t_args));
    watchdog('microstrategy', 'Page %title has been added.', $t_args, WATCHDOG_NOTICE);
  }
  elseif ($form_state['values']['op'] == t('Save page')) {
    //title, path, menu
    db_update('microstrategy_pages')
      ->fields(array(
        'title' => $form_state['values']['name'],
        'path' => $form_state['values']['path'],
        'menu' => $form_state['values']['menu'],
      ))
      ->condition('pid', $form_state['values']['pid'])
      ->execute();
    //roles
    db_delete('microstrategy_pages_roles')
      ->condition('pid', $form_state['values']['pid'])
      ->execute();
    foreach (array_filter($form_state['values']['roles']) as $rid) {
      db_insert('microstrategy_pages_roles')
        ->fields(array(
          'pid' => $form_state['values']['pid'],
          'rid' => $rid,
        ))
        ->execute();
    }
    //log
    $t_args = array('%title' => $form_state['values']['name']);
    drupal_set_message(t('Changes have been saved for page %title.', $t_args));
    watchdog('microstrategy', 'Changes have been saved for page %title.', $t_args, WATCHDOG_NOTICE);
  }

  $form_state['redirect'] = 'admin/structure/microstrategy/page';
  menu_rebuild();
  return;
}

/**
 * Menu callback; displays the MicroStrategy page settings form.
 * $pid MicroStrategy page id.
 */
function microstrategy_page_settings_form($form, &$form_state, $pid) {

  if ($pid == MSTR_PAGE_DEFA_SETT_ID) {
    drupal_set_title(t("Page Default Settings"));
  }
  else {
    drupal_set_title(t("%title settings", array('%title' => _microstrategy_get_page_title($pid))), PASS_THROUGH);
  }

  $config = _microstrategy_get_page_settings($pid);
  $form['pid'] = array('#type' => 'value', '#value' => $pid);

  $form['url'] = _microstrategy_url_fieldset($config);
  $form['server'] = _microstrategy_server_fieldset($config);
  $form['plugins'] = _microstrategy_plugins_fieldset($config);
  $form['parameters'] = _microstrategy_parameters_fieldset($config);
  $form['default_preferences'] = _microstrategy_default_preferences_fieldset($config);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;

}

function microstrategy_page_settings_form_validate($form, &$form_state) {
  module_load_include('inc', 'microstrategy', 'microstrategy.pages');
  _microstrategy_validate_content_settings($form_state);
}

function microstrategy_page_settings_form_submit($form, &$form_state) {

  $pid = $form_state['values']['pid'];

  //close session if server is changed. NOTE this must be performed before saving settings.
  $old_settings = _microstrategy_get_page_settings($pid);
  $server_changed = _microstrategy_server_settings_changed($old_settings, $form_state['values']);
  if ($server_changed) {
    module_load_include('inc', 'microstrategy', 'includes/microstrategy.session');
    if ($pid == MSTR_PAGE_DEFA_SETT_ID) {
      microstrategy_close_all_pages_sessions();
    }
    else {
      microstrategy_close_page_session($pid);
    }
  }

  //save settings
  _microstrategy_save_settings($form_state['values'], $pid, 0, MSTR_TYPE_BASIC);

  //redirect
  if ($pid == MSTR_PAGE_DEFA_SETT_ID) {
    $form_state['redirect'] = 'admin/structure/microstrategy/defaultsettings';
  }
  else {
    $form_state['redirect'] = 'admin/structure/microstrategy/page';
  }

  //log
  if ($pid == MSTR_PAGE_DEFA_SETT_ID) {
    drupal_set_message(t('Page default settings have been saved.'));
    watchdog('microstrategy', 'Page default settings have been saved.', array(), WATCHDOG_NOTICE);
  }
  else {
    $t_args = array('%title' => _microstrategy_get_page_title($pid));
    drupal_set_message(t('%title settings have been saved.', $t_args));
    watchdog('microstrategy', '%title settings have been saved.', $t_args, WATCHDOG_NOTICE);
  }

}

/**
 * Compare form settings with old settings to see if server settings are changed.
 * @param $old_settings old settings.
 * @param $values form state values.
 * @return TRUE if changed, else FALSE.
 */
function _microstrategy_server_settings_changed($old_settings, $values) {
  return ($old_settings['task_url'] != $values['task_url']
    || $old_settings['iserver'] != $values['iserver']
    || $old_settings['port'] != $values['port']
    || $old_settings['sso_namespace'] != $values['sso_namespace']
    || $old_settings['project'] != $values['project']
    || $old_settings['authmode'] != $values['authmode']
    || $old_settings['working_set_size'] != $values['working_set_size']
    || $old_settings['cancel_pending_request'] != $values['cancel_pending_request']
    || $old_settings['remove_history_jobs'] != $values['remove_history_jobs']
  );
}

/**
 * Build a confirm form for deletion of a page.
 */
function microstrategy_page_delete_form($form, &$form_state, $pid) {
  $form['#pid'] = $pid;
  $title = _microstrategy_get_page_title($pid);
  $form['#title'] = $title;
  return confirm_form($form, t('Are you sure you want to delete the page %title?', array('%title' => $title)), 'admin/structure/microstrategy/page/');
}

/**
 * Process page delete form submissions.
 */
function microstrategy_page_delete_form_submit($form, &$form_state) {

  //delete page
  _microstrategy_page_delete($form['#pid']);

  //redirect
  $form_state['redirect'] = 'admin/structure/microstrategy/page/';

  //log
  $t_args = array('%title' => $form['#title']);
  drupal_set_message(t('Page %title has been deleted.', $t_args));
  watchdog('microstrategy', 'Page %title has been deleted.', $t_args, WATCHDOG_NOTICE);

}


////////////////////////////
// Blocks
////////////////////////////

function microstrategy_block_admin_form($form, &$form_state) {
  $form['#theme'] = 'microstrategy_block_admin_form';
  $form['name'] = array(
    '#type' => 'textfield',
    '#size' => 32,
    '#maxlength' => 64,
  );
  $form['type'] = array(
    '#type' => 'select',
    '#default_value' => MSTR_TYPE_BASIC,
    '#options' => array(t('Basic'), t('Master')),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add block'),
  );
  return $form;
}

/**
 * Theme the new-page form.
 *
 * @ingroup themeable
 */
function theme_microstrategy_block_admin_form($variables) {
  $form = $variables['form'];
  $blocks = microstrategy_get_blocks();

  $header = array(t('Name'), t('Type'), array('data' => t('Operations'), 'colspan' => 3));
  $options = array( "query" => drupal_get_destination() );
  if (is_array($blocks) && !empty($blocks)) {
    foreach ($blocks as $block) {
      $title = _microstrategy_get_block_title($block->bid);
      $type = t('Basic');
      if ($block->type == MSTR_TYPE_MASTER) {
        $type = t('Master');
      }
      $configure = l(t('configure'), 'admin/structure/block/manage/microstrategy/'. $block->bid. '/configure', $options);
      $settings = l(t('settings'), 'admin/structure/microstrategy/block/'. $block->bid . '/settings');
      $delete = l(t('delete'), 'admin/structure/microstrategy/block/'. $block->bid . '/delete');
      $rows[] = array(check_plain($title), $type, $configure, $settings, $delete);
    }
  }
  $rows[] = array(drupal_render($form['name']), drupal_render($form['type']), array('data' => drupal_render($form['submit']), 'colspan' => 3));

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  if ($form['form_id'] && $form['form_token']) {
    $output .= drupal_render($form['form_id']);
    $output .= drupal_render($form['form_token']);
  }
  return $output;
}

function microstrategy_block_admin_form_validate($form, &$form_state) {
  if ($form_state['values']['name']) {

  }
  else {
    form_set_error('name', t('You must specify a valid block name.'));
  }
}

function microstrategy_block_admin_form_submit($form, &$form_state) {
  db_insert('microstrategy_blocks')
    ->fields(array(
      'title' => $form_state['values']['name'],
      'type' => $form_state['values']['type'],
      ))
    ->execute();

  $form_state['redirect'] = 'admin/structure/microstrategy/block';

  // update blocks
  module_load_include('inc', 'block', 'block.admin');
  _block_rehash();

  //log
  $t_args = array('%title' => $form_state['values']['name']);
  drupal_set_message(t('Block %title has been added.', $t_args));
  watchdog('microstrategy', 'Block %title has been added.', $t_args, WATCHDOG_NOTICE);
}

/**
 * Menu callback; displays the MicroStrategy block settings form.
 * $bid microstrategy block id. It is 'bid' in microstrategy_blocks table and 'delta' in 'blocks' table.
 */
function microstrategy_block_settings_form($form, &$form_state, $bid) {

  if ($bid == MSTR_BASIC_BLOCK_DEFA_SETT_ID) {
    drupal_set_title(t("Basic block default settings"));
  }
  elseif ($bid == MSTR_MASTER_BLOCK_DEFA_SETT_ID) {
    drupal_set_title(t("Master block default settings"));
  }
  else {
    drupal_set_title(t("%title settings", array('%title' => _microstrategy_get_block_title($bid))), PASS_THROUGH);
  }

  $config = _microstrategy_get_block_settings($bid);
  $block_type = _microstrategy_get_block_type($bid);

  $form['bid'] = array('#type' => 'value', '#value' => $bid);
  $form['block_type'] = array('#type' => 'value', '#value' => $block_type);

  if ($block_type == MSTR_TYPE_BASIC) {
    // basic block
    $form['url'] = _microstrategy_url_fieldset($config);
    $form['server'] = _microstrategy_server_fieldset($config);
    $form['plugins'] = _microstrategy_plugins_fieldset($config);
    $form['parameters'] = _microstrategy_parameters_fieldset($config);
    $form['default_preferences'] = _microstrategy_default_preferences_fieldset($config, $bid);
  }
  elseif ($block_type == MSTR_TYPE_MASTER) {
    // master block
    $form['url'] = _microstrategy_master_url_fieldset($config);
    $form['server'] = _microstrategy_server_fieldset($config);
    $form['plugins'] = _microstrategy_plugins_fieldset($config);
    $form['parameters'] = _microstrategy_parameters_fieldset($config);
    $form['default_preferences'] = _microstrategy_master_default_preferences_fieldset($config, $bid);
    //only show "New Window" for target portlet drop down list.
    $form['master'] = _microstrategy_master_properties_fieldset($config, FALSE);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

function microstrategy_block_settings_form_validate($form, &$form_state) {
  module_load_include('inc', 'microstrategy', 'microstrategy.pages');
  _microstrategy_validate_content_settings($form_state);
}

function microstrategy_block_settings_form_submit($form, &$form_state) {

  $bid = $form_state['values']['bid'];

  //close session if server is changed. NOTE this must be performed before saving settings.
  $old_settings = _microstrategy_get_block_settings($bid);
  $server_changed = _microstrategy_server_settings_changed($old_settings, $form_state['values']);
  if ($server_changed) {
    module_load_include('inc', 'microstrategy', 'includes/microstrategy.session');
    if ($bid == MSTR_BASIC_BLOCK_DEFA_SETT_ID || $bid == MSTR_MASTER_BLOCK_DEFA_SETT_ID) {
      microstrategy_close_all_blocks_sessions();
    }
    else {
      microstrategy_close_block_session($bid);
    }
  }

  //save settings
  _microstrategy_save_settings($form_state['values'], 0, $bid, $form_state['values']['block_type']);

  //redirect
  if ($bid == MSTR_BASIC_BLOCK_DEFA_SETT_ID || $bid == MSTR_MASTER_BLOCK_DEFA_SETT_ID) {
    $form_state['redirect'] = 'admin/structure/microstrategy/defaultsettings';
  }
  else {
    $form_state['redirect'] = 'admin/structure/microstrategy/block';
  }

  //log
  if ($bid == MSTR_BASIC_BLOCK_DEFA_SETT_ID) {
    drupal_set_message(t('Basic block default settings have been saved.'));
    watchdog('microstrategy', 'Basic block default settings have been saved.', array(), WATCHDOG_NOTICE);
  }
  elseif ($bid == MSTR_MASTER_BLOCK_DEFA_SETT_ID) {
    drupal_set_message(t('Master block default settings have been saved.'));
    watchdog('microstrategy', 'Master block default settings have been saved.', array(), WATCHDOG_NOTICE);
  }
  else {
    $t_args = array('%title' => _microstrategy_get_block_title($bid));
    drupal_set_message(t('%title settings have been saved.', $t_args));
    watchdog('microstrategy', '%title settings have been saved.', $t_args, WATCHDOG_NOTICE);
  }

}

/**
 * Build a confirm form for deletion of a block.
 */
function microstrategy_block_delete_form($form, &$form_state, $bid) {
  $form['#bid'] = $bid;
  $title = _microstrategy_get_block_title($bid);
  $form['#title'] = $title;
  return confirm_form($form, t('Are you sure you want to delete the block %title?', array('%title' => $title)), 'admin/structure/microstrategy/block');
}

/**
 * Process block delete form submissions.
 */
function microstrategy_block_delete_form_submit($form, &$form_state) {

  //delete
  _microstrategy_block_delete($form['#bid']);

  //redirect
  $form_state['redirect'] = 'admin/structure/microstrategy/block';

  //log
  $t_args = array('%title' => $form['#title']);
  drupal_set_message(t('Block %title has been deleted.', $t_args));
  watchdog('microstrategy', 'Block %title has been deleted.', $t_args, WATCHDOG_NOTICE);

}

function _microstrategy_url_fieldset($config) {
  $url = array(
    '#type' => 'fieldset',
    '#title' => t('MicroStrategy Content URL'),
    '#collapsible' => TRUE,
  );
  $url['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Web Base URL'),
    '#size' => 60,
    '#default_value' => $config['base_url'],
    '#required' => TRUE,
  );
  $url['task_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Web Task URL'),
    '#size' => 60,
    '#default_value' => $config['task_url'],
    '#required' => TRUE,
  );
  $url['report_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Report URL'),
    '#size' => 60,
    '#default_value' => $config['report_url'],
  );
  $url['folder_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Folder URL'),
    '#size' => 60,
    '#default_value' => $config['folder_url'],
  );
  $url['history_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy History List URL'),
    '#size' => 60,
    '#default_value' => $config['history_url'],
  );
  $url['search_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Search Page URL'),
    '#size' => 60,
    '#default_value' => $config['search_url'],
  );
  $url['project_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Project Page URL'),
    '#size' => 60,
    '#default_value' => $config['project_url'],
  );
  $url['htmldoc_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy HTML Document URL'),
    '#size' => 60,
    '#default_value' => $config['htmldoc_url'],
  );
  $url['doc_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Document URL'),
    '#size' => 60,
    '#default_value' => $config['doc_url'],
  );

  return $url;
}

function _microstrategy_master_url_fieldset($config) {
  $url = array(
    '#type' => 'fieldset',
    '#title' => t('MicroStrategy Content URL'),
    '#collapsible' => TRUE,
  );
  $url['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Web Base URL'),
    '#size' => 60,
    '#default_value' => $config['base_url'],
    '#required' => TRUE,
  );
  $url['task_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Web Task URL'),
    '#size' => 60,
    '#default_value' => $config['task_url'],
    '#required' => TRUE,
  );
  $url['prompt_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Prompt URL'),
    '#size' => 60,
    '#default_value' => $config['prompt_url'],
  );
  $url['folder_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Folder URL'),
    '#size' => 60,
    '#default_value' => $config['folder_url'],
  );
  $url['history_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy History List URL'),
    '#size' => 60,
    '#default_value' => $config['history_url'],
  );
  $url['search_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Search Page URL'),
    '#size' => 60,
    '#default_value' => $config['search_url'],
  );
  $url['report_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Report URL'),
    '#size' => 60,
    '#default_value' => $config['report_url'],
  );
  $url['doc_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Document URL'),
    '#size' => 60,
    '#default_value' => $config['doc_url'],
  );

  return $url;
}

function _microstrategy_server_fieldset($config) {
  $server = array(
    '#type' => 'fieldset',
    '#title' => t('MicroStrategy Server'),
    '#collapsible' => TRUE,
  );
  $server['iserver'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Intelligence Server'),
    '#size' => 60,
    '#default_value' => $config['iserver'],
    '#required' => TRUE,
  );
  $server['port'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Intelligence Server Port'),
    '#size' => 60,
    '#default_value' => $config['port'],
    '#required' => TRUE,
  );
  $server['sso_namespace'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Single Sign-on Namespace'),
    '#size' => 60,
    '#default_value' => $config['sso_namespace'],
    '#description' => t('All MicroStrategy pages and blocks with the same Single Sign-on Namespace share the same MicroStrategy Credentials for this user.'),
  );
  $server['authmode'] = array(
    '#type' => 'select',
    '#title' => t('MicroStrategy Authentication Mode'),
    '#default_value' => $config['authmode'],
    '#options' => array(
      DssXmlAuthStandard => t('Standard'),
      DssXmlAuthNTCredential => t('NT'),
      DssXmlAuthAnonymous => t('Guest'),
      DssXmlAuthLDAP => t('LDAP'),
      DssXmlAuthWarehousePassthrough => t('Database'),
      DssXmlAuthSimpleSecurityPlugIn => t('Trusted'),
      DssXmlAuthIntegrated => t('Integrated')
    ),
  );
  $server['show_locale'] = array(
    '#type' => 'checkbox',
    '#title' => t('Let User Select Locales for MicroStrategy Web'),
    '#default_value' => $config['show_locale'],
  );
  $server['project'] = array(
    '#type' => 'textfield',
    '#title' => t('MicroStrategy Project Name'),
    '#size' => 60,
    '#default_value' => $config['project'],
    '#required' => TRUE,
  );
  $server['working_set_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Regular Working Set Size'),
    '#size' => 60,
    '#default_value' => $config['working_set_size'],
  );
  $server['cancel_pending_request'] = array(
    '#type' => 'checkbox',
    '#title' => t('Cancel pending requests upon logout'),
    '#default_value' => $config['cancel_pending_request'],
  );
  $server['remove_history_jobs'] = array(
    '#type' => 'select',
    '#title' => t('Remove jobs in History List upon logout'),
    '#default_value' => $config['remove_history_jobs'],
    '#options' => array(t('No'), t('Yes'), t('ReadOnly') ),
  );

  return $server;
}

function _microstrategy_plugins_fieldset($config) {
  $output = array(
    '#type' => 'fieldset',
    '#title' => t('Plug-ins'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $output['credential_mapper_file'] = array(
    '#type' => 'textfield',
    '#title' => t('Credential Mapper File Name'),
    '#size' => 60,
    '#default_value' => isset($config['credential_mapper_file']) ? $config['credential_mapper_file'] : NULL,
  );
  $output['credential_mapper_function'] = array(
    '#type' => 'textfield',
    '#title' => t('Credential Mapper Function Name'),
    '#size' => 60,
    '#default_value' => isset($config['credential_mapper_function']) ? $config['credential_mapper_function'] : NULL,
  );
  $output['parameter_mapper_file'] = array(
    '#type' => 'textfield',
    '#title' => t('URL Parameter Mapper File Name'),
    '#size' => 60,
    '#default_value' => isset($config['parameter_mapper_file']) ? $config['parameter_mapper_file'] : NULL,
  );
  $output['parameter_mapper_function'] = array(
    '#type' => 'textfield',
    '#title' => t('URL Parameter Mapper Function Name'),
    '#size' => 60,
    '#default_value' => isset($config['parameter_mapper_function']) ? $config['parameter_mapper_function'] : NULL,
  );
  return $output;
}

function _microstrategy_parameters_fieldset($config) {
  $output = array(
    '#type' => 'fieldset',
    '#title' => t('User Parameters'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $output['shared_parameter'] = array(
    '#type' => 'textfield',
    '#title' => t('Shared User Parameter'),
    '#size' => 60,
    '#default_value' => isset($config['shared_parameter']) ? $config['shared_parameter'] : NULL,
    '#description' => t('Format of "name1:value1;name2:value2;"'),
  );
  $output['personal_parameter_template'] = array(
    '#type' => 'textfield',
    '#title' => t('Personal User Parameter Template'),
    '#size' => 60,
    '#default_value' => isset($config['personal_parameter_template']) ? $config['personal_parameter_template'] : NULL,
    '#description' => t('Format of "name1:display name1:default value;name2: display name2:default value2;"'),
  );
  return $output;
}

function _microstrategy_default_preferences_fieldset($config, $bid=0) {
  module_load_include('inc', 'microstrategy', 'microstrategy.pages');
  $output = array(
    '#type' => 'fieldset',
    '#title' => t('Default Preferences'),
    '#collapsible' => TRUE,
    //'#collapsed' => TRUE,
  );
  $output['content'] = _microstrategy_content_fieldset($config, $bid);
  $output['display'] = _microstrategy_display_fieldset($config);
  return $output;
}

function _microstrategy_master_default_preferences_fieldset($config, $bid=0) {
  module_load_include('inc', 'microstrategy', 'microstrategy.pages');
  $output = array(
    '#type' => 'fieldset',
    '#title' => t('Default Preferences'),
    '#collapsible' => TRUE,
    //'#collapsed' => TRUE,
  );
  $output['content'] = _microstrategy_master_content_fieldset($config, $bid);
  $output['display'] = _microstrategy_display_fieldset($config);
  return $output;
}

function _microstrategy_save_settings($values, $pid, $bid, $block_type) {

  $config = new stdClass();
  $config->pid = $pid;
  $config->bid = $bid;

  $config->base_url = $values['base_url'];
  $config->task_url = $values['task_url'];

  $config->report_url = $values['report_url'];
  $config->folder_url = $values['folder_url'];
  $config->history_url = $values['history_url'];
  $config->search_url = $values['search_url'];
  $config->doc_url = $values['doc_url'];

  if ($block_type == MSTR_TYPE_BASIC) {
    $config->project_url = $values['project_url'];
    $config->htmldoc_url = $values['htmldoc_url'];
  }
  elseif ($block_type == MSTR_TYPE_MASTER) {
    $config->prompt_url = $values['prompt_url'];
  }

  $config->iserver = $values['iserver'];
  $config->port = $values['port'];
  $config->sso_namespace = $values['sso_namespace'];
  $config->authmode = $values['authmode'];
  $config->project = $values['project'];
  $config->show_locale = $values['show_locale'];
  $config->working_set_size = $values['working_set_size'];
  $config->cancel_pending_request = $values['cancel_pending_request'];
  $config->remove_history_jobs = $values['remove_history_jobs'];
  $config->shared_parameter = $values['shared_parameter'];
  $config->personal_parameter_template = $values['personal_parameter_template'];

  //plug-ins
  $config->credential_mapper_file = $values['credential_mapper_file'];
  $config->credential_mapper_function = $values['credential_mapper_function'];
  $config->parameter_mapper_file = $values['parameter_mapper_file'];
  $config->parameter_mapper_function = $values['parameter_mapper_function'];

  // default preferences
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

  // Remove any old settings
  db_delete('microstrategy_settings')
    ->condition('pid', $config->pid)
    ->condition('bid', $config->bid)
    ->execute();

  // Update settings
  drupal_write_record('microstrategy_settings', $config);

}

/**
 * Validation helper function for page path
 *
 * @param $path
 *   The path to be tested for the page
 * @param $pid
 *   The id of the page we're checking
 * @return
 *   TRUE if path is valid to use, otherwise, FALSE.
 */
function _microstrategy_check_path($path, $pid) {

  // Ensure path fits the rules:
  if (preg_match('/[^-a-z0-9_\/]/', $path)) {
    form_set_error('path', t('Path must be lowercase alphanumeric, underscores, dashes, or forward-slashes only.'));
    return FALSE;
  }

  // Check path for preceeding or trailing forward slashes
  if (substr($path, 0, 1) == '/' ||
    substr($path, strlen($path) - 1, 1) == '/') {
    form_set_error('path', t('Path cannot begin or end with a slash.'));
    return FALSE;
  }

  // Check path against existing MicroStrategy paths
  $pages = db_query("SELECT * FROM {microstrategy_pages}");
  foreach ($pages as $page) {
    // If this is the page we're checking, skip it
    if ($pid && $pid == $page->pid) {
      continue;
    }
    if ($page->path == $path) {
      form_set_error('path', t('The chosen path is already in use.'));
      return FALSE;
    }
  }

  return TRUE;
}
