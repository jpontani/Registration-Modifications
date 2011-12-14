<?php
/**
 * @file
 *   Registration rules file.
 */

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

function registration_mark_confirmed($registration_id) {
  db_update("registration")
    ->fields("status", "confirm")
    ->condition('registration_id', $registration_id)
    ->execute();
}