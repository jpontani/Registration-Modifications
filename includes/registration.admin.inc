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
        $status = variable_get('registration_status:' . $type . ':_' . $bundle, 0);
        $usingtype = variable_get('registration_bundle:' . $type . ':_' . $bundle, '');
        $status = ($status ? 'Enabled' : 'Disabled');
        $status = l($status, 'admin/structure/registration/status/' . $type . '/' . $bundle) . ($status == 'Enabled' ? ' <small>(' . $usingtype . ')</small>' : '');
        if (!empty($bundle_data['admin'])) {
          if (!empty($bundle_data['admin']['real path'])) {
            $link = l($bundle_data['label'], $bundle_data['admin']['real path']);
          }
          else {
            $link = l($bundle_data['label'], $bundle_data['admin']['path']);
          }
        }
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
      unset($record->created);
      $ret = drupal_write_record('registration_type', $record, array('type'));
      $msg = ' ' . t('updated.');
      field_attach_rename_bundle('registration', $form_state['values']['bundle_old'], $form_state['values']['bundle']);
      break;
  }
  entity_info_cache_clear();
  registration_entity_types_reset();
  drupal_set_message(t('Registration entity <em>@type</em> has been @msg.'), array('@type' => $record->type, '@msg' => $msg));
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
  //$args = func_get_arg(1);
  //$type = $args['build_info']['args'][0];
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
  $form['actions']['cancel'] = array(
    '#type' => 'button',
    '#value' => t('Cancel'),
    '#submit' => array('registration_delete_type_form_cancel'),
    '#executes_submit_callback' => FALSE,
  );
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete @type', array('@type' => $type)),
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
      // delete all field values attached to each registration of this type
      $reg = registration_load($id);
      field_attach_delete('registration', $reg);
      entity_delete('registration', $id);
    }
  }
  // notify field api that this registration bundle is being deleted
  field_attach_delete_bundle('registration', $reg_type);
  // we reset all entities that use this registration bundle to no bundle and
  // set registration status to disabled
  registration_reset_status_with_type($reg_type);
  // go ahead and delete the type from the DB
  db_delete('registration_type')->condition('type', $reg_type)->execute();
  // reset our cache of registration bundles
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
 * Callback for cancel delete button click.
 */
function registration_delete_type_form_cancel($form, $form_state) {
  drupal_goto('admin/structure/registration/types');
}

/**
 * Registration entity edit/delete form title callback.
 */
function registration_admin_form_title($type, $action, $bundle = NULL) {
  $mtype = registration_type_load($type);
  switch ($action) {
    case 'Status':
      return 'Set Status for ' . $type . ':' . $bundle;
    case 'Delete':
      return 'Delete type ' . $mtype->name;
    case 'Edit':
      return 'Edit type ' . $mtype->name;
  }
}

/**
 * Entity registration status form callback.
 */
function registration_status_form($form, $form_state, $type, $bundle) {
  $form = array();

  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $type . ':_' . $bundle,
  );

  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#default_value' => variable_get('registration_status:' . $type . ':_' . $bundle, 0),
    '#options' => array('Disabled', 'Enabled'),
  );

  $form['registration_entity'] = array(
    '#type' => 'select',
    '#title' => t('Registration Type'),
    '#default_value' => variable_get('registration_bundle:' . $type . ':_' . $bundle, ''),
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
  $type = $form_state['values']['type'];
  variable_set('registration_bundle:' . $type, $form_state['values']['registration_entity']);
  variable_set('registration_status:' . $type, $form_state['values']['status']);
  drupal_set_message(t('Registration settings updated for @type.', array('@type' => $type)));
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