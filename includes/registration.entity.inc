<?php

/**
 * @file
 * Entity hooks and callbacks for registrations.
 */

/**
 * Implements hook_entity_info().
 */
function registration_entity_info() {
  $entities = array(
    'registration' => array(
      'module' => 'registration',
      'label' => t('Registration'),
      'controller class' => 'EntityAPIController',
      'entity class' => 'RegistrationEntityClass',
      'base table' => 'registration',
      'fieldable' => TRUE,
      'entity keys' => array(
        'id' => 'registration_id',
        'bundle' => 'type',
      ),
      'bundle keys' => array(
        'bundle' => 'type',
      ),
      'bundles' => array(),
      'view modes' => array(
        'full' => array(
          'label' => t('Full Registration'),
          'custom settings' => FALSE,
        ),
      ),
      'uri callback' => 'registration_uri',
      'token type' => 'registration',
    ),
  );

  foreach (registration_entity_types() as $type => $info) {
    $entities['registration']['bundles'][$type] = array(
      'label' => $info->name,
      'admin' => array(
        'path' => 'admin/structure/registration/types/manage/' . $info->type,
        'access arguments' => array('administer registration'),
      ),
    );
  }

  return $entities;
}

function registration_uri($registration) {
  return array(
    'path' => 'registration/' . $registration->registration_id,
  );
}

/**
 * Main class for test entities.
 */
class RegistrationEntityClass extends Entity {

  public function __construct(array$values = array(), $entityType = NULL) {
    parent::__construct($values, $entityType);
  }

  // Override buildContent() to add registration properties.
  public function buildContent($view_mode = 'full', $langcode = NULL) {
    // invoke theme handler to return markup for registration properties
    $content['registration'] = array(
      '#markup' => theme('registration', array('registration' => $this)),
    );

    return entity_get_controller($this->entityType)
      ->buildContent($this, $view_mode, $langcode, $content);
  }
}

/**
 * Implements hook_entity_property_info_alter().
 */
function registration_entity_property_info_alter(&$info) {
  $properties = &$info['registration']['properties'];

  // Convert timestamps props to dates
  $properties['created'] = array(
    'label' => t("Created"),
    'description' => t("The date the registration was created."),
    'type' => 'date',
    'schema field' => 'created',
  );
  $properties['updated'] = array(
    'label' => t("Updated"),
    'description' => t("The date the registration was updated."),
    'type' => 'date',
    'schema field' => 'updated',
  );

  return $info;
}

/**
 * Returns all the bundles for the registration entity.
 */
function registration_entity_types() {
  $types = &drupal_static(__FUNCTION__);
  if (empty($types)) {
    // types array is empty, so lets load it
    $types = array();

    // Get any custom module registration bundles from the hook and add them to
    // the database or merge them with existing entries.
    $hooks = module_invoke_all('registration_type_info');
    foreach ($hooks as $bundle => $bundle_info) {
      db_merge('registration_type', 'rt')
        ->fields(array(
          'name' => $bundle_info['title'],
          'type' => $bundle,
          'description' => $bundle_info['description'],
          'created' => REQUEST_TIME,
          'changed' => REQUEST_TIME,
          ))
        ->key(array('type' => $bundle))
        ->updateExcept('created')
        ->execute();
    }

    $tq = db_select('registration_type', 'rt')->fields('rt')->execute()->fetchAssoc();
    foreach ($tq as $row) {
      // Let modules alter the registration bundle info.
      drupal_alter('registration_type_info', $row);
      $type = $row['type'];
      $types[$type] = $row;
    }
  }
  return $types;
}

/**
 * Resets the cached list of registration types.
 */
function registration_entity_types_reset() {
  $registration_types = &drupal_static('registration_entity_types');
  $registration_types = NULL;
}

/**
 * Loads a registration type.
 */
function registration_type_load($type) {
  $types = registration_entity_types();
  $type = str_replace('-', '_', $type);
  return isset($types[$type]) ? $types[$type] : FALSE;
}

/**
 * Loads a registration by ID.
 */
function registration_load($registration_id) {
  if (empty($registration_id)) {
    return FALSE;
  }

  $registrations = registration_load_multiple(array($registration_id), array());
  return $registrations ? reset($registrations) : FALSE;
}

/**
 * Loads multiple registrations by ID or based on a set of matching conditions.
 *
 * @see entity_load()
 *
 * @param $registration_ids
 * @param $conditions
 *   An array of conditions on the {registration} table in the form
 *     'field' => $value.
 * @param $reset
 *   Whether to reset the internal registration loading cache.
 *
 * @return
 *   An array of contact objects indexed by registration_id.
 */
function registration_load_multiple($registration_ids = array(), $conditions = array(), $reset = FALSE) {
  if (empty($registration_ids) && empty($conditions)) {
    return array();
  }

  return entity_load('registration', $registration_ids, $conditions, $reset);
}

/**
 * Deletes multiple registrations by ID.
 *
 * @param $registration_ids
 *   An array of registration IDs to delete.
 *
 * @return
 *   TRUE on success, FALSE otherwise.
 */
function registration_delete_multiple($registration_ids) {
  // Let modules act on a registration about to be deleted.
  if (is_array($registration_ids)) {
    foreach ($registration_ids as $regid) {
      $reg = registration_load($regid);
      module_invoke_all('registration_delete', $reg);
    }
  }
  else {
    $reg = registration_load($registration_ids);
    module_invoke_all('registration_delete', $reg);
  }
  return entity_get_controller('registration')->delete($registration_ids);
}

/**
 * Saves a registration.
 *
 * @param $registration
 *   The full registration object to save.
 *
 * @return
 *   The saved registration object.
 */
function registration_save($registration) {
  $registration->updated = REQUEST_TIME;
  // Set the creation timestamp if not set.
  if (!isset($registration->created) || empty($registration->created)) {
    $registration->created = REQUEST_TIME;
  }

  $ret = entity_get_controller('registration')->save($registration);

  // Let other modules act on a registration insert or update.
  if (!isset($registration->registration_id)) {
    module_invoke_all('registration_insert', $registration);
  }
  else {
    module_invoke_all('registration_update', $registration);
  }

  return $ret;
}

/**
 * Implements hook_field_extra_fields().
 */
function registration_field_extra_fields() {
  $extra = array();

  foreach (registration_entity_types() as $type => $reg_type) {
    $extra['registration'][$type] = array(
      'form' => array(
        'email' => array(
          'label' => t('Email'),
          'description' => t('Registration module email form element'),
          'weight' => -5,
        ),
      ),
      'display' => array(
        'email' => array(
          'label' => t('Email'),
          'description' => t("Registered User's Email Address"),
          'weight' => -5,
        ),
      ),
    );
  }

  return $extra;
}