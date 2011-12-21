<?php
/**
 * @file
 *   Registration rules file.
 */

/**
 * Implements hook_rules_event_info().
 */
function registration_rules_event_info() {
  $events['registration_insert'] = array(
    'group' => t('Registration'),
    'label' => t('After creating a registration'),
    'variables' => array(
      'registration' => array(
        'label' => t('A registration'),
        'type' => 'registration',
      ),
    ),
  );
  $events['registration_update'] = array(
    'group' => t('Registration'),
    'label' => t('After saving an existing registration'),
    'variables' => array(
      'registration' => array(
        'label' => t('A registration'),
        'type' => 'registration',
      ),
    ),
  );
  $events['registration_delete'] = array(
    'group' => t('Registration'),
    'label' => t('Before deleting a registration'),
    'variables' => array(
      'registration' => array(
        'label' => t('A registration'),
        'type' => 'registration',
      ),
    ),
  );

  return $events;
}

/**
 * Implements hook_rules_action_info().
 */
function registration_rules_action_info() {
  $actions = array();

  $actions['registration_mark_registration_confirm'] = array(
    'label' => t('Mark Registration as confirmed'),
    'group' => t('Registration'),
    'parameter' => array(
      'registration_id' => array(
        'type' => 'integer',
        'label' => t('Registration ID'),
        'description' => t('The Registration ID to mark as confirmed.'),
      ),
    ),
    'callbacks' => array(
      'execute' => 'registration_mark_confirmed',
    ),
  );
  return $actions;
}

/**
 * Rules action callback.
 */
function registration_mark_confirmed($registration_id) {
  db_update("registration")
    ->fields("status", "confirm")
    ->condition('registration_id', $registration_id)
    ->execute();
}