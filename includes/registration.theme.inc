<?php
/**
 * @file
 *  Contains the theme hooks for the Registration module.
 */

/**
 * Themes a specific registration type row.
 */
function theme_registration_types($variables) {
  $type = $variables['type'];

  $output = check_plain($type->name);
  $output .= ' <small> (Machine name: ' . check_plain($type->type) . ')</small>';
  $output .= '<div class="description">' . filter_xss_admin($type->description) . '</div>';

  return $output;
}

/**
 * Theme handler for registrations.
 *
 * @param array $variables
 *  Contains a complete registration object.
 */
function theme_registration($variables) {
  $registration = $variables['registration'];
  //if ($node = node_load($registration->nid)) {
  //  $output .= '<div><label>' . $node->type . "</label>" . l($node->title, 'node/' . $registration->nid) . '</div>';
  //}

  $output = '<div><label>' . t('Count') . '</label>' . $registration->count . '</div>';
  $output .= '<div><label>' . t('Created') . '</label>' . format_date($registration->created) . '</div>';

  return $output;
}