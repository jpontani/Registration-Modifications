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

  $output = "<div><label>Registration #" . $registration->registration_id . "</label></div>";
  $output .= '<div><label>' . t('Email Address') . '</label>' . $reg->mail . '</div>';
  $output .= '<div><label>' . t('Created') . '</label>' . format_date($registration->created) . '</div>';
  $output .= '<hr><div>' . field_attach_view('registration', $reg, 'full') . '</div>';

  return $output;
}