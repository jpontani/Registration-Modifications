<?php
/**
 * @file
 * Registration status field views handler.
 */

class registration_handler_field_registration_status extends views_handler_field {
  function render($values) {
    $q = db_select("registration_status", 'rs')
      ->fields('rs', array('status_name'))
      ->condition('status_key', $values->registration_status)
      ->execute()
      ->fetchCol();
    return $q[0];
  }
}