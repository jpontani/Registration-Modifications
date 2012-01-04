<?php
/**
 * @file
 * Registration token implementation.
 */

/**
 * Implements hook_token_info().
 */
function registration_token_info() {
  return array(
    'types' => array('registration' => array(
      'name' => t('Registration'),
      'description' => t('Tokens related to Registrations.')
      'needs-data' => 'registration',
    )),
    'tokens' => array('registration' => array(
      'view' => array(
        'name' => t('Full Registration'),
        'description' => t('Displays the complete registration object.'),
      ),
      'url' => array(
        'name' => t('Registration Link'),
        'description' => t('A link to view the registration.'),
      ),
      'id' => array(
        'name' => t('Registration ID'),
        'description' => t('The registration ID number.'),
      ),
      'mail' => array(
        'name' => t('Registration Email'),
        'description' => t('The email address associated with this registration.'),
      ),
      'status' => array(
        'name' => t('Registration Status'),
        'description' => t('The status of the registration.'),
      ),
      'date' => array(
        'name' => t('Registration Date'),
        'description' => t('The date the registration was created.'),
      ),
      'date:custom' => array(
        'name' => t('Registration Date Custom'),
        'description' => t('Custom formatted registration date.'),
      ),
      'entity_id' => array(
        'name' => t('Registration Entity ID'),
        'description' => t('The ID of the entity this registration is for.'),
      ),
      'entity_type' => array(
        'name' => t('Registration Entity Type'),
        'description' => t('The type of the entity this registration is for.'),
      ),
      'entity_bundle' => array(
        'name' => t('Registration Entity Bundle'),
        'description' => t('The bundle of the entity this registration is for.'),
      ),
      'entity' => array(
        'name' => t('Registration Entity'),
        'description' => t('Displays the complete entity this registration is for.'),
      ),
      'entity_title' => array(
        'name' => t('Registration Entity Title'),
        'description' => t('The title of the entity this registration is for.'),
      ),
      'entity_link' => array(
        'name' => t('Registration Entity Link'),
        'description' => t('A link to view the entity this registration is for.'),
      ),
    )),
  );
}

/**
 * Implements hook_tokens().
 */
function registration_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $items = array();
  if (!isset($data['registration'])) {
    return array();
  }
  $reg = $data['registration'];
  if ($type == "registration") {
    foreach ($tokens as $key => $token) {
      $value = "";
      switch ($key) {
        case "view":
          $value = theme('registration', array('registration' => $reg));
          break;
        case "url":
          $uri = entity_uri('registration', $reg);
          $value = url($uri['path'], $uri['options']);
          break;
        case "id":
          $value = $reg->registration_id;
          break;
        case "mail":
          $value = $reg->mail;
          break;
        case "status":
          $q = db_select('registration_status', 'rs')
            ->fields('rs', array('status_name'))
            ->condition('status_key', $reg->status)
            ->execute()
            ->fetchCol();
          $value = $q[0];
          break;
        case "date":
          $value = date("r", $reg->created;);
          break;
        case "date:custom":
          $value = date("", $reg->created);
          break;
        case "entity_id":
          $value = $reg->eid;
          break;
        case "entity_type":
          $value = $reg->entity_type;
          break;
        case "entity_bundle":
          $value = $reg->entity_bundle;
          break;
        case "entity":
          $value = "";
          break;
        case "entity_title":
          $entity = entity_load($reg->entity_type, $reg->eid);
          $value = entity_label($reg->entity_type, $entity);
          break;
        case "entity_link":
          $entity = entity_load($reg->entity_type, $reg->eid);
          $uri = entity_uri($reg->entity_type, $entity);
          $value = url($uri['path'], $uri['options']);
          break;
      }
      $items[$token] = $value;
    }
  }
  return $items;
}