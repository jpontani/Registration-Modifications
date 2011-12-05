<?php
/**
 * @file
 *  Contains form definitions and handlers for Registration.
 */

/**
 * Form callback: create or edit a contact.
 *
 * @param $contact
 *   The contact object to edit or for a create form an empty contact object
 *   with only a contact type defined.
 */
function registration_form($form, &$form_state, $registration, $entity, $type) {
  $form_state['registration'] = $registration;
  $form_state['entity'] = $entity;
  $form_state['entity_type'] = $type;
  
  $data = entity_extract_ids($type, $entity);

  $entity_info = array(
    'id' => $data[0],
    'type' => $type,
    'bundle' => $data[2],
  );

  global $user;
  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#description' => t('The email to associate with this registration.'),
    '#default_value' => $user->uid ? $user->mail : '',
    '#size' => 40,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $settings  = registration_entity_settings($entity_info);
  $capacity  = $settings['capacity'];
  $remaining = 20;
  if ($capacity) {
    $remaining = $capacity - registration_event_count($entity_info);
  }
  $options = array();
  for ($i = 1; $i < $remaining + 1; $i++) {
    $options[$i] = $i;
  }
  $form['count'] = array(
    '#type' => 'select',
    '#title' => t('Count'),
    '#description' => t('Select the number of registrations you want to create.'),
    '#options' => $options,
    '#default_value' => 1,
    '#access' => isset($settings['multiple_registrations']) ?
        $settings['multiple_registrations'] : FALSE,
  );


  field_attach_form('registration', $registration, $form, $form_state);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Registration'),
  );

  // add a delete button for existing registration
  if (isset($registration->registration_id)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );
  }

  if (!empty($registration->registration_id) &&
    user_access('administer registration')
  ) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('registration_form_delete_submit'),
    );
  }

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'node/' . $data[0],
  );

  return $form;
}

/**
 * Validation callback for registration_form().
 */
function registration_form_validate($form, &$form_state) {
  $entity = $form_state['entity'];
  $type = $form_state['entity_type'];
  $registration = $form_state['registration'];
  $data = entity_extract_ids($type, $entity);
  $entity_info = array(
    'id' => $data[0],
    'type' => $type,
    'bundle' => $data[2],
  );
  // verify the event hasn't sold out
  if (!registration_has_room($entity_info)) {
    drupal_set_message(t('Sorry, this event has sold out during your registration.'));
  }

  // validate email address
  if (!valid_email_address($form_state['values']['email'])) {
    form_set_error('email', t('The email address is invalid.'));
  }
  // @TODO check for duplicate registrations
  else {
    $count = 0;
    if ($count > 0) {
      form_set_error('email', t('%mail is already registered for this event.',
        array('%mail' => $form_state['values']['email'])));
    }
  }

  // Notify field widgets to validate their data.
  field_attach_form_validate('registration', $registration, $form, $form_state);
}

/**
 * Submit callback for registration_form().
 */
function registration_form_submit($form, &$form_state) {
  $entity = $form_state['entity'];
  $type = $form_state['entity_type'];
  $registration = $form_state['registration'];
  $data = entity_extract_ids($type, $entity);

  // Set the contact's author uid
  global $user;
  $registration->author_uid = $user->uid;
  $email = $form_state['values']['email'];
  $registration->count = $form_state['values']['count'];

  // Notify field widgets.
  field_attach_submit('registration', $registration, $form, $form_state);

  // Save the registration and redirect to the tour node.
  if (registration_save($registration)) {
    drupal_set_message(t('Thanks you, your registration has been recieved!'));
    $form_state['redirect'] = 'node/' . $data[0];
  }
  else {
    drupal_set_message(t('Sorry, there has been a problem submitting your registration.'));
  }
}

/**
 * Button submit function: handle the 'Delete' button on the node form.
 */
function registration_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $registration = $form_state['registration'];
  $form_state['redirect'] = array('registration/' .
    $registration->registration_id . '/delete', array('query' => $destination),
  );
}

/**
 * Menu callback -- ask for confirmation of node deletion
 */
function registration_delete_confirm($form, &$form_state, $registration) {
  // Always provide entity id in the same form key as in the entity edit form.
  $form['registration'] = array(
    '#type' => 'value',
    '#value' => $registration,
  );
  return confirm_form($form,
    t('Are you sure you want to delete registration %title?',
      array('%title' => $registration->registration_id)
    ),
    'registration/' . $registration->registration_id,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Execute node deletion
 */
function registration_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $registration = $form_state['values']['registration'];
    registration_delete_multiple(array($registration->registration_id));
    watchdog('registration', 'Registration %id deleted.', array('%id' => $registration->registration_id));
    drupal_set_message(t('Registration %id has been deleted.', array('%id' => $registration->registration_id)));
  }

  $form_state['redirect'] = 'node/' . $registration->nid;
}

/**
 * Return a form for sending a broadcast email to participants.
 */
function registration_registrations_broadcast_form($form, &$form_state, $node) {
  $form_state['node'] = $node;

  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#description' => t('The subject of the message.'),
    '#required' => TRUE,
    '#size' => 40,
    '#maxlength' => 40,
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#description' => t('Enter the message you want to send to this tour\'s participants.'),
    '#required' => TRUE,
    '#cols' => 60,
    '#rows' => 5,
  );
  $form['send'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );

  return $form;
}

/**
 * Submit handler for registration_registrations_broadcast_form.
 */
function registration_registrations_broadcast_form_submit($form, &$form_state) {
  registration_send_broadcast(
    $form_state['node'],
    $form_state['values']['subject'],
    $form_state['values']['message']
  );
}

/**
 * Return a form for a node's registration settings.
 *
 * @param array $form
 * @param array $form_state
 * @param object $node
 */
function registration_registrations_settings_form($form, &$form_state, $entity, $type) {
  $form_state['entity'] = $entity;
  $form_state['entity_type'] = $type;
  
  $data = entity_extract_ids($type, $entity);
  
  $entity_info = array(
    'id' => $data[0],
    'type' => $type,
    'bundle' => $data[2],
  );

  $settings = registration_entity_settings($entity_info);
  
  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable'),
    '#description' => t('Check to enable registrations for %name.',
      array('%name' => $entity->title)
    ),
    '#default_value' => isset($settings['status']) ? $settings['status'] : -1,
  );
  $form['capacity'] = array(
    '#type' => 'textfield',
    '#title' => t('Capacity'),
    '#description' => t('The maximum number of regsitrants for %name. Leave at 0
         for no limit.', array('%name' => $entity->title)),
    '#size' => 5,
    '#maxlength' => 10,
    '#required' => TRUE,
    '#default_value' => isset($settings['capacity']) ? $settings['capacity'] : 0,
  );

  $form['reminder'] = array(
    '#type' => 'fieldset',
    '#title' => t('Reminder'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['reminder']['send_reminder'] = array(
    '#type' => 'checkbox',
    '#title' => 'Send Reminder',
    '#description' => 'If checked, a reminder will be sent to registrants on the following date.',
    '#default_value' => isset($settings['send_reminder']) ? $settings['send_reminder'] : -1,
  );

  $reminder_date = array();
  if (isset($settings['reminder_date'])) {
    $datetime = strtotime($settings['reminder_date']);
    $reminder_date = array(
      'year' => date('Y', $datetime),
      'month' => date('n', $datetime),
      'day' => date('d', $datetime),
    );
  }

  $form['reminder']['reminder_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#collapsible' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="send_reminder"]' => array('checked' => TRUE),
      ),
    )
  );
  $form['reminder']['reminder_settings']['reminder_date'] = array(
    '#type' => 'date',
    '#title' => 'Reminder Date',
    '#default_value' => $reminder_date,
  );
  $form['reminder']['reminder_settings']['reminder_template'] = array(
    '#type' => 'textarea',
    '#title' => 'Template',
    '#default_value' => isset($settings['reminder_template']) ? $settings['reminder_template'] : '',
    '#description' => t('The reminder message sent to registrants. Tokens are supported if the module is enabled, E.g., [node:title].'),
  );
  // add token support
  if (module_exists('token')) {
    $form['reminder']['reminder_settings']['token_tree'] = array(
      '#theme' => 'token_tree',
      '#token_types' => array('node', 'content-type', 'current-page', 'registration'),
      '#global_types' => FALSE,
    );
  }

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Additional Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );
  $form['settings']['multiple_registrations'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow multiple registrations'),
    '#description' => t('If selected, users can register for more than one slot for this event.'),
    '#default_value' => isset($settings['multiple_registrations']) ? $settings['multiple_registrations'] : -1,
  );
  $form['settings']['from_address'] = array(
    '#type' => 'textfield',
    '#title' => t('From Address'),
    '#description' => t('From email address to use for confirmations, reminders, and broadcast emails.'),
    '#required' => TRUE,
    '#default_value' => isset($settings['from_address']) ?
      $settings['from_address'] :
      variable_get('site_mail', ini_get('sendmail_from')),
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Settings'),
  );

  return $form;
}

/**
 * Submit handler for registration_registrations_settings_form().
 *
 * @param array $form
 * @param array $form_state
 */
function registration_registrations_settings_form_validate($form, &$form_state) {
  // ensure capacity is a positive integer
  $capacity = $form_state['values']['capacity'];
  if (!is_numeric($capacity) || ((int)$capacity != $capacity) || ($capacity < 0)) {
    form_set_error('capacity', t('Capacity must be a positive integer.'));
  }

  // validate from address
  if (!valid_email_address($form_state['values']['settings']['from_address'])) {
    form_set_error('from', t('From email address is invalid.'));
  }
}

/**
 * Submit handler for registration_registrations_settings_form().
 *
 * @param array $form
 * @param array $form_state
 */
function registration_registrations_settings_form_submit($form, &$form_state) {
  $entity = $form_state['entity'];
  $type = $form_state['entity_type'];
  $data = entity_extract_ids($type, $entity);
  $info = array(
    'id' => $data[0],
    'type' => $type,
    'bundle' => $data[2],
  );
  $fields = array(
    'entity_id' => $data[0],
    'entity_type' => $type,
    'entity_bundle' => $data[2],
    'settings' => array(
      'status' => $form_state['values']['status'],
      'capacity' => $form_state['values']['capacity'],
      'send_reminder' => $form_state['values']['send_reminder'],
    ) + $form_state['values']['settings'],
  );
  if ($form_state['values']['send_reminder']) {
    $fields['settings']['reminder_date'] = $form_state['values']['reminder_date']['year']
      . '-' . $form_state['values']['reminder_date']['month']
      . '-' . $form_state['values']['reminder_date']['day'];
    $fields['settings']['reminder_template'] = $form_state['values']['reminder_template'];
  }
  $fields['settings'] = serialize($fields['settings']);
  registration_update_entity_settings($info, $fields);
}

/**
 * Register button form callback.
 *
 * Shows a Register button for a registration enabled entity.
 *
 * If the user is already registered for this item, we instead show text to the
 * user that they are already registered.
 */
function registration_register_form($form, &$form_state, $type, $bundle, $eid) {
  global $user;
  $form = array();
  if ($user->uid != 0 && registration_check_email_registered($user->mail, $type, $bundle, $eid)) {
    return array(
      'already' => array(
        '#type' => 'markup',
        '#markup' => '<small>* ' . t('You are already registered for this item.') . ' *</small>',
      ),
    );
  }
  $form['eid'] = array(
    '#type' => 'hidden',
    '#value' => $eid,
  );
  $form['etype'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );
  $form['bundle'] = array(
    '#type' => 'hidden',
    '#value' => $bundle,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Register',
  );
  return $form;
}

/**
 * Register button form submit.
 *
 * Loads up the complete registration form for the entity.
 */
function registration_register_form_submit($form, &$form_state) {
  drupal_goto('register/' . $form_state['values']['etype'] . '/' . $form_state['values']['bundle'] . '/' . $form_state['values']['eid']);
}

/**
 * Registration form page title callback.
 */
function registration_register_entity_title($type, $bundle, $entityid) {
  $entity = entity_load($type, array($entityid));
  return t('Register for @title', array('@title' => $entity[$entityid]->title));
}

/**
 * Form builder for registering for an entity. Adds bundle's fields to the form.
 */
function registration_register_entity_form($form, &$form_state, $type, $bundle, $entityid) {
  $info = array(
    'id' => $entityid,
    'type' => $type,
    'bundle' => $bundle,
  );
  $entity = registration_entity_registration_bundle($info);
  $status = registration_entity_registration_status($info);
  $settings = registration_entity_settings($info);
  $form = array();
  $form['entity_type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );
  $form['entity_bundle'] = array(
    '#type' => 'hidden',
    '#value' => $bundle,
  );
  $form['entity_id'] = array(
    '#type' => 'hidden',
    '#value' => $entityid,
  );
  if ($status == 0) {
    $form += array(
      'invalid' => array(
        '#type' => 'markup',
        '#markup' => t('This item is not accepting registrations at this time.'),
      ),
      'actions' => array(
        '#type' => 'actions',
        'goback' => array(
          '#type' => 'button',
          '#value' => t('Go Back'),
        ),
      ),
    );
    return $form;
  }
  $open_date = $settings['settings']['open_date'];
  $close_date = $settings['settings']['close_date'];
  if (REQUEST_TIME >= $open_date && REQUEST_TIME <= $close_date && is_string($entity)) {
    $etype = $entity;
    $entity = entity_create('registration', array('type' => $etype));
    $entity->entity_type = $type;
    $entity->entity_bundle = $bundle;
    $entity->eid = $entityid;
    $form['registration'] = array(
      '#type' => 'fieldset',
      '#title' => 'Registration Information',
    );
    $form['registration']['entity'] = array(
      '#type' => 'value',
      '#value' => $entity,
    );
    $form['registration']['email'] = array(
      '#type' => 'textfield',
      '#text_format' => 'email_default',
      '#title' => 'Email Address',
      '#required' => TRUE,
    );
    $form['actions'] = array(
      '#type' => 'actions',
      'cancel' => array(
        '#type' => 'button',
        '#value' => t('Cancel'),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Submit Registration'),
      ),
    );
    field_attach_form('registration', $entity, $form['registration'], $form_state);
    return $form;
  }
  else {
    $form += array(
      'invalid' => array(
        '#type' => 'markup',
        '#markup' => t('Registrations for this item are currently closed.'),
      ),
      'actions' => array(
        '#type' => 'actions',
        'goback' => array(
          '#type' => 'button',
          '#value' => t('Go Back'),
        ),
      ),
    );
    return $form;
  }
}

/**
 * Registration form validation callback.
 */
function registration_register_entity_form_validate($form, &$form_state) {
  $entityid = $form_state['values']['entity_id'];
  $entitytype = $form_state['values']['entity_type'];
  $entitybundle = $form_state['values']['entity_bundle'];
  $registration = $form_state['values']['entity'];
  field_attach_form_validate('registration', $registration, $form, $form_state);
  return TRUE;
}

/**
 * Registration form submit callback.
 */
function registration_register_entity_form_submit($form, &$form_state) {
  global $user;
  $entityid = $form_state['values']['entity_id'];
  $entitytype = $form_state['values']['entity_type'];
  $entitybundle = $form_state['values']['entity_bundle'];
  $registration = $form_state['values']['entity'];
  $registration->created = REQUEST_TIME;
  $registration->updated = REQUEST_TIME;
  $registration->user_uid = $user->uid;
  $registration->author_uid = $user->uid;
  $registration->status = 1;
  entity_save('registration', $registration);
  field_attach_submit('registration', $registration, $form['registration'], $form_state);
  drupal_set_message(t('Your registration has been received.'));
  drupal_goto($entitytype . '/' . $entityid);
}