<?php

/**
 * @file
 * Page callbacks for the Contact module.
 */

use Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException;
use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;

/**
 * Page callback: Form constructor for the site-wide contact form.
 *
 * @throws \Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException
 * @throws \Symfony\Component\HttpKernel\Exception\NotFoundHttpException
 *
 * @see contact_menu()
 * @see contact_site_form_submit()
 * @ingroup forms
 */
function contact_site_form($form, &$form_state) {
  global $user;

  // Check if flood control has been activated for sending e-mails.
  $config = config('contact.settings');
  $limit = $config->get('flood.limit');
  $interval = $config->get('flood.interval');
  if (!drupal_container()->get('flood')->isAllowed('contact', $limit, $interval) && !user_access('administer contact forms')) {
    drupal_set_message(t("You cannot send more than %limit messages in @interval. Try again later.", array('%limit' => $limit, '@interval' => format_interval($interval))), 'error');
    throw new AccessDeniedHttpException();
  }

  // Get an array of the categories and the current default category.
  $categories = entity_load_multiple('contact_category');

  // If there are no categories, do not display the form.
  if (!$categories) {
    if (user_access('administer contact forms')) {
      drupal_set_message(t('The contact form has not been configured. <a href="@add">Add one or more categories</a> to the form.', array('@add' => url('admin/structure/contact/add'))), 'error');
    }
    else {
      throw new NotFoundHttpException();
    }
  }

  // Prepare array for select options.
  uasort($categories, 'Drupal\Core\Config\Entity\ConfigEntityBase::sort');
  $options = array();
  foreach ($categories as $category) {
    $options[$category->id()] = $category->label();
  }

  // If there is more than one category available and no default category has
  // been selected, prepend a default placeholder value.
  $default_category = $config->get('default_category');
  if (!$default_category) {
    $default_category = !empty($categories) ? key($categories) : NULL;
  }

  if (!$user->uid) {
    $form['#attached']['library'][] = array('system', 'jquery.cookie');
    $form['#attributes']['class'][] = 'user-info-from-cookie';
  }

  $form['#attributes']['class'][] = 'contact-form';
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Your name'),
    '#maxlength' => 255,
    '#default_value' => $user->uid ? user_format_name($user) : '',
    '#required' => TRUE,
  );
  $form['mail'] = array(
    '#type' => 'email',
    '#title' => t('Your e-mail address'),
    '#default_value' => $user->uid ? $user->mail : '',
    '#required' => TRUE,
  );

  // Do not allow authenticated users to alter the name or e-mail values to
  // prevent the impersonation of other users.
  if ($user->uid) {
    // Hide the original name and e-mail address fields and display read-only
    // versions in their place.
    $form['name']['#access'] = $form['mail']['#access'] = FALSE;
    $form['name_display'] = array(
      '#type' => 'item',
      '#title' => t('Your name'),
      '#markup' => check_plain($form['name']['#default_value']),
    );
    $form['mail_display'] = array(
      '#type' => 'item',
      '#title' => t('Your e-mail address'),
      '#markup' => check_plain($form['mail']['#default_value']),
    );
  }
  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['category'] = array(
    '#type' => 'select',
    '#title' => t('Category'),
    '#default_value' => $default_category,
    '#options' => $options,
    '#empty_value' => 0,
    '#required' => TRUE,
    '#access' => count($categories) > 1,
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#required' => TRUE,
  );
  // Do not allow anonymous users to send themselves a copy because it can be
  // abused to spam people.
  $form['copy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send yourself a copy.'),
    '#access' => $user->uid,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send message'),
  );

  return $form;
}

/**
 * Form submission handler for contact_site_form().
 */
function contact_site_form_submit($form, &$form_state) {
  global $user;
  $language_interface = language(LANGUAGE_TYPE_INTERFACE);

  $values = $form_state['values'];
  $values['sender'] = $user;
  $values['sender']->name = $values['name'];
  $values['sender']->mail = $values['mail'];
  $values['category'] = entity_load('contact_category', $values['category']);

  if (!$user->uid) {
    $values['sender']->name .= ' (' . t('not verified') . ')';
    // Save the anonymous user information to a cookie for reuse.
    user_cookie_save(array_intersect_key($values, array_flip(array('name', 'mail'))));
  }

  // Get the to and from e-mail addresses.
  $to = implode(', ', $values['category']->recipients);
  $from = $values['sender']->mail;

  // Send the e-mail to the recipients using the site default language.
  drupal_mail('contact', 'page_mail', $to, language_default()->langcode, $values, $from);

  // If the user requests it, send a copy using the current language.
  if ($values['copy']) {
    drupal_mail('contact', 'page_copy', $from, $language_interface->langcode, $values, $from);
  }

  // Send an auto-reply if necessary using the current language.
  if ($values['category']->reply) {
    drupal_mail('contact', 'page_autoreply', $from, $language_interface->langcode, $values, $to);
  }

  drupal_container()->get('flood')->register('contact', config('contact.settings')->get('flood.interval'));
  watchdog('mail', '%sender-name (@sender-from) sent an e-mail regarding %category.', array('%sender-name' => $values['name'], '@sender-from' => $from, '%category' => $values['category']->label()));

  // Jump to home page rather than back to contact page to avoid
  // contradictory messages if flood control has been activated.
  drupal_set_message(t('Your message has been sent.'));
  $form_state['redirect'] = '';
}

/**
 * Page callback: Form constructor for the personal contact form.
 *
 * @param $recipient
 *   The account for which a personal contact form should be generated.
 *
 * @throws \Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException
 *
 * @see contact_menu()
 * @see contact_personal_form_submit()
 *
 * @ingroup forms
 */
function contact_personal_form($form, &$form_state, $recipient) {
  global $user;

  // Check if flood control has been activated for sending e-mails.
  $config = config('contact.settings');
  $limit = $config->get('flood.limit');
  $interval = $config->get('flood.interval');
  if (!drupal_container()->get('flood')->isAllowed('contact', $limit, $interval) && !user_access('administer contact forms') && !user_access('administer users')) {
    drupal_set_message(t("You cannot send more than %limit messages in @interval. Try again later.", array('%limit' => $limit, '@interval' => format_interval($interval))), 'error');
    throw new AccessDeniedHttpException();
  }

  drupal_set_title(t('Contact @username', array('@username' => user_format_name($recipient))), PASS_THROUGH);

  if (!$user->uid) {
    $form['#attached']['library'][] = array('system', 'jquery.cookie');
    $form['#attributes']['class'][] = 'user-info-from-cookie';
  }

  $form['#attributes']['class'][] = 'contact-form';
  $form['recipient'] = array(
    '#type' => 'value',
    '#value' => $recipient,
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Your name'),
    '#maxlength' => 255,
    '#default_value' => $user->uid ? user_format_name($user) : '',
    '#required' => TRUE,
  );
  $form['mail'] = array(
    '#type' => 'email',
    '#title' => t('Your e-mail address'),
    '#default_value' => $user->uid ? $user->mail : '',
    '#required' => TRUE,
  );
  // Do not allow authenticated users to alter the name or e-mail values to
  // prevent the impersonation of other users.
  if ($user->uid) {
    // Hide the original name and e-mail address fields and display read-only
    // versions in their place.
    $form['name']['#access'] = $form['mail']['#access'] = FALSE;
    $form['name_display'] = array(
      '#type' => 'item',
      '#title' => t('Your name'),
      '#markup' => check_plain($form['name']['#default_value']),
    );
    $form['mail_display'] = array(
      '#type' => 'item',
      '#title' => t('Your e-mail address'),
      '#markup' => check_plain($form['mail']['#default_value']),
    );
  }
  $form['to'] = array(
    '#type' => 'item',
    '#title' => t('To'),
    '#markup' => theme('username', array('account' => $recipient)),
  );
  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#maxlength' => 50,
    '#required' => TRUE,
  );
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#rows' => 15,
    '#required' => TRUE,
  );
  // Do not allow anonymous users to send themselves a copy
  // because it can be abused to spam people.
  $form['copy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send yourself a copy.'),
    '#access' => $user->uid,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send message'),
  );
  return $form;
}

/**
 * Form submission handler for contact_personal_form().
 */
function contact_personal_form_submit($form, &$form_state) {
  global $user;
  $language_interface = language(LANGUAGE_TYPE_INTERFACE);

  $values = $form_state['values'];
  $values['sender'] = $user;
  $values['sender']->name = $values['name'];
  $values['sender']->mail = $values['mail'];

  if (!$user->uid) {
    $values['sender']->name .= ' (' . t('not verified') . ')';
    // Save the anonymous user information to a cookie for reuse.
    user_cookie_save(array_intersect_key($values, array_flip(array('name', 'mail'))));
  }

  // Get the to and from e-mail addresses.
  $to = $values['recipient']->mail;
  $from = $values['sender']->mail;

  // Send the e-mail in the requested user language.
  drupal_mail('contact', 'user_mail', $to, user_preferred_langcode($values['recipient']), $values, $from);

  // Send a copy if requested, using current page language.
  if ($values['copy']) {
    drupal_mail('contact', 'user_copy', $from, $language_interface->langcode, $values, $from);
  }

  drupal_container()->get('flood')->register('contact', config('contact.settings')->get('flood.interval'));
  watchdog('mail', '%sender-name (@sender-from) sent %recipient-name an e-mail.', array('%sender-name' => $values['name'], '@sender-from' => $from, '%recipient-name' => $values['recipient']->name));

  // Jump to the contacted user's profile page.
  drupal_set_message(t('Your message has been sent.'));
  $form_state['redirect'] = user_access('access user profiles') ? 'user/' . $values['recipient']->uid : '';
}
