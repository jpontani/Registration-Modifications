<?php

class registration_handler_field_registration_entity extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['eid'] = 'eid';
  }

  function render($values) {
    $product = "";
    return $product;
  }
}