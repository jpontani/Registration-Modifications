<?php
/**
 * @file
 *   Registration views integration.
 */

/**
 * Implements hook_views_data().
 */
function registration_views_data() {
  $tables = array();

  // Registrations table.
  $tables['registration']['table']['group'] = t('Registration');
  $tables['registration']['table']['base'] = array(
    'field' => 'registration_id',
    'title' => t('Registration'),
    'help' => t('A registration entity'),
  );
  $tables['registration']['table']['entity type'] = 'registration';
  $tables['registration']['table']['join'] = array(
    'registration_status' => array(
      'left_field' => 'status_key',
      'field' => 'status',
    ),
    'registration_type' => array(
      'left_field' => 'type',
      'field' => 'type',
    ),
    'commerce_order' => array(
      'left_field' => 'order_id',
      'field' => 'order_id',
    ),
  );
  $tables['registration']['registration_id'] = array(
    'title' => t('Registration ID Number'),
    'help' => t('The unique identifier for the registration.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric', 
      'click sortable' => TRUE,
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  $tables['registration']['type'] = array(
    'title' => t('Registration Bundle'), 
    'help' => t('The registration bundle used.'), 
    'field' => array(
      'handler' => 'views_handler_field', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ), 
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $tables['registration']['entity_type'] = array(
    'title' => t('Attached Entity Type'), 
    'help' => t('The base type of the entity this registration is attached to (node, user, etc.).'), 
    'field' => array(
      'handler' => 'views_handler_field', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ), 
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $tables['registration']['entity_bundle'] = array(
    'title' => t('Attached Entity Bundle'), 
    'help' => t('The bundle of the entity this registration is attached to.'), 
    'field' => array(
      'handler' => 'views_handler_field', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ), 
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $tables['registration']['eid'] = array(
    'title' => t('Attached Entity ID'), 
      'help' => t('Unique identifier of the entity this registration is attached to.'), 
    'field' => array(
      'handler' => 'views_handler_field_numeric', 
      'click sortable' => TRUE,
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );
  $tables['registration']['mail'] = array(
    'title' => t('Registration Email'), 
    'help' => t('The email this registration uses.'), 
    'field' => array(
      'handler' => 'views_handler_field', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ), 
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $tables['registration']['count'] = array(
    'title' => t('Registration Count'), 
      'help' => t('The number of slots this registration uses.'), 
    'field' => array(
      'handler' => 'views_handler_field_numeric', 
      'click sortable' => TRUE,
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $tables['registration']['author_uid'] = array(
    'title' => t('User ID'),
    'help' => t('User ID of the registration submitter.'),
    'field' => array(
      'handler' => 'views_handler_field_user', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_user_uid',
    ),

  );
  $tables['registration']['created'] = array(
    'title' => t('Created Date Timestamp'), 
    'help' => t('Timestamp of when the registration was created.'), 
    'field' => array(
      'handler' => 'views_handler_field_date', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  $tables['registration']['updated'] = array(
    'title' => t('Update Date Timestamp'), 
    'help' => t('Timestamp of when the registration was last updated.'), 
    'field' => array(
      'handler' => 'views_handler_field_date', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );
  $tables['registration']['order_id'] = array(
    'relationship' => array(
      'base' => 'commerce_order',
      'field' => 'order_id', 
      'handler' => 'views_handler_relationship',
      'label' => t('Commerce Order'),
    ),
    'title' => t('Commerce Order ID'), 
    'help' => t('The Commerce Order ID Number.'), 
    'field' => array(
      'handler' => 'views_handler_field_numeric', 
      'click sortable' => TRUE,
    ), 
    'sort' => array(
      'handler' => 'views_handler_sort',
    ), 
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // Registration status table.

  return $tables;
}