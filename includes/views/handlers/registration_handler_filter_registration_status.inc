<?php
/**
 * @file
 * Registration registration status views filter handler.
 */

class registration_handler_filter_registration_status extends views_handler_filter {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Registration Status');
      $q = db_select('registration_status', 'rs')
        ->fields('rs')
        ->execute()
        ->fetchAllAssoc();
      $this->value_options = $q;
    }
  }
}