<?php
/**
 * @file
 *   Provides hooks for Registrations.
 */

/**
 * Implements hook_hook_info().
 */
function registration_hook_info() {
  $hooks = array(
    'registration_default_settings' => array(
      'group' => 'registration',
    ),
    'registration_type_info' => array(
      'group' => 'registration',
    ),
    'registration_type_info_alter' => array(
      'group' => 'registration',
    ),
    'registration_insert' => array(
      'group' => 'registration',
    ),
    'registration_update' => array(
      'group' => 'registration',
    ),
    'registration_delete' => array(
      'group' => 'registration',
    ),
  );

  return $hooks;
}