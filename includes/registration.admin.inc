<?php
/**
 * @file
 *  Contains admin UI functions for Registration.
 */

/**
 * Page callback to list all registration types.
 */
function registration_types_page() {
  $header = array(t('Name'), t('Operations'));
  $rows = array();
  foreach (registration_entity_types() as $type => $info) {
    $links = array(
      'edit' => array(
        'title' => 'edit',
        'href' => 'admin/structure/registration/types/manage/' . $type,
      ),
      'manage fields' => array(
        'title' => 'manage fields',
        'href' => 'admin/structure/registration/types/manage/' . $type . '/fields',
      ),
      'manage display' => array(
        'title' => 'manage display',
        'href' => 'admin/structure/registration/types/manage/' . $type . '/display',
      ),
      'delete' => array(
        'title' => 'delete',
        'href' => 'admin/structure/registration/types/' . $type . '/delete',
      ),
    );
    $rows[] = array(
      theme('registration_types', array('type' => $info)),
      theme('links', array('links' => $links, 'attributes' => array('class' => 'links inline operations'))),
    );
  }

  if (empty($rows)) {
    $rows[] = array(
      array(
        'data' => t('There are no registration types yet. <a href="@link">Add a Registration Type</a>', array('@link' => url('admin/structure/registration/types/add'))),
        'colspan' => 2,
      ),
    );
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Landing page for managing the structure of registrations.
 */
function registration_admin_page() {
  $rows = array();
  $entities = entity_get_info();
  foreach ($entities as $type => $data) {
    if (!empty($data['bundles']) && $type != 'registration') {
      $rows[] = array(
        array(
          'data' => '<b>' . $data['label'] . '</b>',
          'colspan' => 2,
        ),
      );
      foreach ($data['bundles'] as $bundle => $bundle_data) {
        $link = $bundle_data['label'];
        $info = array(
          'id' => 0,
          'type' => $type,
          'bundle' => $bundle,
        );
        $status = registration_entity_registration_status($info);
        $usingtype = registration_entity_registration_bundle($info);
        $status = ($status ? 'Enabled' : 'Disabled');
        $status = l($status, 'admin/structure/registration/status/' . $type . '/' . $bundle) . ($status == 'Enabled' ? ' <small>(' . $usingtype . ')</small>' : '');
        $rows[] = array($link, $status);
      }
    }
  }

  $table = array(
    'header' => array(t('Entity Type'), t('Registration Status')),
    'rows' => $rows,
  );
  $out = theme('table', $table);

  return $out;
}

/**
 * Form callback for adding or editing a registration type.
 */
function registration_type_form($form, &$form_state, $type = NULL) {
  $name = '';
  $machine = '';
  $desc = '';
  $op = 'new';
  $mtype = FALSE;
  $form = array();

  if (is_string($type)) {
    $mtype = registration_type_load($type);
  }

  if ($mtype) {
    $name = $mtype->name;
    $machine = $mtype->type;
    $desc = $mtype->description;
    $op = 'edit';
    $form['bundle_old'] = array(
      '#type' => 'hidden',
      '#value' => $machine,
    );
  }

  $form['pop'] = array(
    '#type' => 'hidden',
    '#default_value' => $op,
    '#value' => $op,
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Name',
    '#default_value' => $name,
    '#required' => TRUE,
  );
  $form['bundle'] = array(
    '#type' => 'machine_name',
    '#title' => 'Machine name',
    '#machine_name' => array(
      'exists' => 'registration_entity_type_exists',
    ),
    '#default_value' => $machine,
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => 'Description',
    '#default_value' => $desc,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  return $form;
}

/**
 * Submit handler for adding or editing a registration type.
 */
function registration_type_form_submit($form, $form_state) {
  $op = $form_state['values']['pop'];
  $msg = '';
  $record = (object) array(
    'name' => $form_state['values']['name'],
    'type' => $form_state['values']['bundle'],
    'description' => $form_state['values']['description'],
    'created' => REQUEST_TIME,
    'changed' => REQUEST_TIME,
  );
  $ret = '';
  switch ($op) {
    case 'new':
      $ret = drupal_write_record('registration_type', $record);
      $msg = ' ' . t('created.');
      field_attach_create_bundle('registration', $form_state['values']['bundle']);
      break;
    case 'edit':
      $old = $form_state['values']['bundle_old'];
      unset($record->created);
      db_update('registration_type')
        ->condition('type', $old)
        ->fields(
          array(
            'name' => $record->name,
            'type' => $record->type,
            'description' => $record->description,
            'changed' => $record->changed,
          )
        )
        ->execute();
      $msg = ' ' . t('updated.');
      field_attach_rename_bundle('registration', $form_state['values']['bundle_old'], $form_state['values']['bundle']);
      break;
  }
  entity_info_cache_clear();
  registration_entity_types_reset();
  drupal_set_message(t('Registration entity <em>@type</em> has been @msg', array('@type' => $record->type, '@msg' => $msg)));
  drupal_goto('admin/structure/registration/types');
}

/**
 * Machine name existence callback.
 */
function registration_entity_type_exists($type) {
  $types = registration_entity_types();
  if (array_key_exists($type, $types)) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Delete form callback.
 *
 * @todo Delete all registrations of that type.
 * @todo Delete all variables that have this type as the registration entity.
 * @todo Delete all fields associated with only this entity.
 */
function registration_delete_type_form($form, $form_state, $type) {
  $form = array();
  $form['delete_confirm'] = array(
    '#type' => 'markup',
    '#markup' => t("<h2>Are you sure you wish to delete <em>@type?</em> This action cannot be undone!</h2>", array('@type' => $type)),
  );
  $form['reg_type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete @type', array('@type' => $type)),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'markup',
    '#markup' => l('Cancel', '/admin/structure/registration/types'),
  );
  return $form;
}

/**
 * Delete submit handler for registration type.
 */
function registration_delete_type_form_submit($form, $form_state) {
  $reg_type = $form_state['values']['reg_type'];
  $regs = new EntityFieldQuery;
  $regs = $regs->entityCondition('entity_type', 'registration')
    ->propertyCondition('type', $reg_type)
    ->execute();
  foreach ($regs as $entity_type => $entities) {
    foreach ($entities as $id => $entity) {
      // Delete all field values attached to each registration of this type.
      $reg = registration_load($id);
      field_attach_delete('registration', $reg);
      entity_delete('registration', $id);
    }
  }
  // Notify field api that this registration bundle is being deleted.
  field_attach_delete_bundle('registration', $reg_type);
  // We reset all entities that use this registration bundle to no bundle and
  // set registration status to disabled.
  registration_reset_status_with_type($reg_type);
  // Go ahead and delete the type from the DB.
  db_delete('registration_type')
    ->condition('type', $reg_type)
    ->execute();
  // Delete entity attachments with this registration bundle.
  db_delete('registration_attach')
    ->condition('registration_bundle', $reg_type)
    ->execute();
  // Reset our cache of registration bundles.
  registration_entity_types_reset();
  drupal_set_message(t('The registration bundle <em>@type</em> has been deleted.', array('@type' => $reg_type)));
  drupal_goto('admin/structure/registration/types');
}

/**
 * Disable all entities with this registration bundle.
 *
 * Also resets the associated registration bundle to blank for any entity that
 * uses the given bundle.
 *
 * @param $reg_type
 *   Registration bundle to check for and reset any entities that use this.
 */
function registration_reset_status_with_type($reg_type) {
  $items = db_select('variable', 'v')->fields('v')->condition('name', 'registration_bundle:%', 'LIKE')->execute();
  foreach ($items as $row) {
    $value = variable_get($row->name, '');
    if ($value == $reg_type) {
      variable_set($row->name, '');
      $bundle_key = 'registration_status:' . substr($row->name, 20);
      variable_set($bundle_key, 0);
    }
  }
}

/**
 * Registration entity edit/delete form title callback.
 */
function registration_admin_form_title($type, $action, $bundle = NULL) {
  $mtype = registration_type_load($type);
  $t = "Msg";
  switch ($action) {
    case 'Status':
      $t = t('Set Status for @bundle', array('@bundle' => $type . ':' . $bundle));
      break;
    default:
      $t = t('@action type @type', array('@action' => $action, '@type' => $mtype->name));
      break;
  }
  return $t;
}

/**
 * Entity registration status form callback.
 */
function registration_status_form($form, $form_state, $type, $bundle) {
  $info = array(
    'id' => 0,
    'type' => $type,
    'bundle' => $bundle,
  );
  $form = array();

  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );

  $form['bundle'] = array(
    '#type' => 'hidden',
    '#value' => $bundle,
  );

  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#default_value' => registration_entity_registration_status($info),
    '#options' => array('Disabled', 'Enabled'),
  );

  $form['registration_entity'] = array(
    '#type' => 'select',
    '#title' => t('Registration Type'),
    '#default_value' => registration_entity_registration_bundle($info),
    '#options' => registration_bundle_options(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Settings',
  );

  return $form;
}

/**
 * Entity registration status submit handler.
 */
function registration_status_form_submit($form, &$form_state) {
  $bundle = $form_state['values']['bundle'];
  $type = $form_state['values']['type'];
  $entity = array(
    'id' => 0,
    'type' => $type,
    'bundle' => $bundle,
  );
  $settings = array(
    'registration_bundle' => $form_state['values']['registration_entity'],
    'settings' => serialize(registration_default_settings()),
  );
  registration_update_entity_settings($entity, $settings);
  drupal_goto('admin/structure/registration');
}

/**
 * Callback to get options list of registration bundles.
 */
function registration_bundle_options() {
  $options = array('' => 'Select a Bundle');
  foreach (registration_entity_types() as $type => $info) {
    $options[$type] = $info->name;
  }
  return $options;
}

/**
 * Registration default settings landing page.
 */
function registration_default_settings_form($form, &$form_state) {
  $form = registration_default_settings_base_form();

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save Defaults'),
    ),
    'cancel' => array(
      '#type' => 'markup',
      '#markup' => l('Cancel', '/admin/structure/registration/entities'),
    ),
  );
  return $form;
}

/**
 * Returns the base form fields for registration settings.
 *
 * @param $parent
 *   Array of form parents, if applicable. This allows form nesting for use on
 *   other forms outside of the Registration module.
 */
function registration_default_settings_base_form($parent = array(), $settings = array()) {
  if (empty($settings)) {
    $settings = variable_get('registration_default_settings', array());
  }
  $mime = !module_exists('mime_mail');
  $mimemsg = t('(requires the Mime Mail module)');
  
  $commerce = module_exists('commerce_email');

  // Basic entity registration settings.
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Basic Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#parents' => array_merge($parent, array('settings')),
  );
  $form['settings']['status'] = array(
    '#parents' => array_merge($parent, array('settings', 'status')),
    '#type' => 'checkbox',
    '#title' => t('Allow Registrations'),
    '#default_value' => $settings['settings']['status'],
  );
  $form['settings']['capacity'] = array(
    '#parents' => array_merge($parent, array('settings', 'capacity')),
    '#type' => 'textfield',
    '#title' => t('Capacity'),
    '#description' => t('Setting capacity to 0 removes the registration limit.'),
    '#default_value' => $settings['settings']['capacity'],
  );
  $form['settings']['waitlist'] = array(
    '#parents' => array_merge($parent, array('settings', 'waitlist')),
    '#type' => 'checkbox',
    '#title' => t('Waitlist'),
    '#default_value' => $settings['settings']['waitlist'],
  );
  $form['settings']['waitlist_capacity'] = array(
    '#parents' => array_merge($parent, array('settings', 'waitlist_capacity')),
    '#type' => 'textfield',
    '#title' => t('Waitlist Capacity'),
    '#description' => t('Setting capacity to 0 removes the waitlist registration limit.'),
    '#default_value' => $settings['settings']['waitlist_capacity'],
  );
  $form['settings']['open_date'] = array(
    '#parents' => array_merge($parent, array('settings', 'open_date')),
    '#type' => 'textfield',
    '#title' => t('Open Date'),
    '#description' => t('Default open date format, as utilized by PHP\'s !link function.',
      array('!link' => l('strtotime()', 'http://php.net/manual/en/function.strtotime.php'))),
    '#default_value' => $settings['settings']['open_date'],
  );
  $form['settings']['close_date'] = array(
    '#parents' => array_merge($parent, array('settings', 'close_date')),
    '#type' => 'textfield',
    '#title' => t('Close Date'),
    '#description' => t('Default close date format, as utilized by PHP\'s !link function.',
      array('!link' => l('strtotime()', 'http://php.net/manual/en/function.strtotime.php'))),
    '#default_value' => $settings['settings']['close_date'],
  );

  // Notification settings.
  $form['notifications'] = array(
    '#type' => 'fieldset',
    '#title' => t('Notification Settings'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#collapsed' => !$settings['notifications']['send_notification'],
    '#parents' => array_merge($parent, array('notifications')),
  );
  $form['notifications']['send_notification'] = array(
    '#parents' => array_merge($parent, array('notifications', 'send_notification')),
    '#title' => t('Send Notification Emails'),
    '#type' => 'checkbox',
    '#default_value' => $settings['notifications']['send_notification'],
  );
  if (!$commerce) {
    $form['notifications']['subject'] = array(
      '#parents' => array_merge($parent, array('notifications', 'subject')),
      '#title' => t('Notification Email Subject'),
      '#type' => 'textfield',
      '#default_value' => $settings['notifications']['subject'],
    );
    $form['notifications']['template'] = array(
      '#parents' => array_merge($parent, array('notifications', 'template')),
      '#title' => t('Notification Email Template'),
      '#type' => 'textarea',
      '#default_value' => $settings['notifications']['template'],
    );
    $form['notifications']['template_file'] = array(
      '#parents' => array_merge($parent, array('notifications', 'template_file')),
      '#title' => t('Notification Email Template File'),
      '#type' => 'select',
      '#options' => registration_template_options(),
      '#default_value' => $settings['notifications']['template_file'],
    );
    $form['notifications']['html'] = array(
      '#parents' => array_merge($parent, array('notifications', 'html')),
      '#title' => t('Send Notification Email as HTML'),
      '#description' => $mimemsg,
      '#type' => 'checkbox',
      '#disabled' => $mime,
      '#default_value' => $settings['notifications']['html'],
    );
  }
  else {
    $form['notifications']['commerce_notification'] = array(
      '#type' => 'markup',
      '#markup' => l('Notification Email Settings from Commerce Email', 'admin/commerce/config/email'),
    );
  }

  // Reminder settings.
  $form['reminders'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Reminder Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => !$settings['reminders']['send_reminder'],
    '#parents' => array_merge($parent, array('reminders')),
  );
  $form['reminders']['send_reminder'] = array(
    '#parents' => array_merge($parent, array('reminders', 'send_reminder')),
    '#title' => t('Send Reminder Emails'),
    '#type' => 'checkbox',
    '#default_value' => $settings['reminders']['send_reminder'],
  );
  if (!$commerce) {
    $form['reminders']['from_address'] = array(
      '#parents' => array_merge($parent, array('reminders', 'from_address')),
      '#title' => t('Reminder Email From Address'),
      '#type' => 'textfield',
      '#default_value' => $settings['reminders']['from_address'],
    );
    $form['reminders']['subject'] = array(
      '#parents' => array_merge($parent, array('reminders', 'subject')),
      '#title' => t('Reminder Email Subject'),
      '#type' => 'textfield',
      '#default_value' => $settings['reminders']['subject'],
    );
    $form['reminders']['template'] = array(
      '#parents' => array_merge($parent, array('reminders', 'template')),
      '#title' => t('Reminder Email Template'),
      '#type' => 'textarea',
      '#default_value' => $settings['reminders']['template'],
    );
    $form['reminders']['template_file'] = array(
      '#parents' => array_merge($parent, array('reminders', 'template_file')),
      '#title' => t('Email Template File'),
      '#type' => 'select',
      '#options' => registration_template_options(),
      '#default_value' => $settings['reminders']['template_file'],
    );
    $form['reminders']['html'] = array(
      '#parents' => array_merge($parent, array('reminders', 'html')),
      '#title' => t('Send Reminder Email as HTML') . $mimemsg,
      '#type' => 'checkbox',
      '#disabled' => $mime,
      '#description' => $mimemsg,
      '#default_value' => $settings['reminders']['html'],
    );
  }
  else {
    $form['reminders']['commerce_reminder'] = array(
      '#type' => 'markup',
      '#markup' => l('Reminder Email Settings from Commerce Email', 'admin/commerce/config/email'),
    );
  }
  return $form;
}

/**
 * Registration default settings form validate handler.
 */
function registration_default_settings_form_validate($form, &$form_state) {
}

/**
 * Registration default settings form submit handler.
 */
function registration_default_settings_form_submit($form, &$form_state) {
  $settings = array();
  foreach ($form_state['values'] as $key => $value) {
    if (is_array($form_state['values'][$key])) {
      $settings[$key] = $value;
    }
  }
  variable_set('registration_default_settings', $settings);
  drupal_set_message(t('Registration default settings saved.'));
  drupal_goto('admin/structure/registration/entities');
}

/**
 * Returns list of email template files.
 *
 * @return
 *  Array of email template filenames keyed by URI.
 */
function registration_template_options() {
  $base_dir = drupal_get_path('module', 'registration') . '/templates';
  $files = file_scan_directory($base_dir, '/.*\.tpl\.php$/');
  $file_array = array('' => "Don't Use a Template File");
  foreach ($files as $uri => $file) {
    $file_array[$file->uri] = $file->filename;
  }
  return $file_array;
}