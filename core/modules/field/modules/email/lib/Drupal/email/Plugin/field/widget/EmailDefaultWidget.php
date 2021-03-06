<?php

/**
 * @file
 * Definition of Drupal\email\Plugin\field\widget\EmailDefaultWidget.
 */

namespace Drupal\email\Plugin\field\widget;

use Drupal\Core\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\field\Plugin\Type\Widget\WidgetBase;

/**
 * Plugin implementation of the 'email_default' widget.
 *
 * @Plugin(
 *   id = "email_default",
 *   module = "email",
 *   label = @Translation("E-mail"),
 *   field_types = {
 *     "email"
 *   },
 *   settings = {
 *     "placeholder" = ""
 *   }
 * )
 */
class EmailDefaultWidget extends WidgetBase {

  /**
   * Implements Drupal\field\Plugin\Type\Widget\WidgetInterface::settingsForm().
   */
  public function settingsForm(array $form, array &$form_state) {
    $element['placeholder'] = array(
      '#type' => 'textfield',
      '#title' => t('Placeholder'),
      '#default_value' => $this->getSetting('placeholder'),
      '#description' => t('The placeholder is a short hint (a word or short phrase) intended to aid the user with data entry. A hint could be a sample value or a brief description of the expected format.'),
    );
    return $element;
  }

  /**
   * Implements Drupal\field\Plugin\Type\Widget\WidgetInterface::formElement().
   */
  public function formElement(array $items, $delta, array $element, $langcode, array &$form, array &$form_state) {
    $element['value'] = $element + array(
      '#type' => 'email',
      '#default_value' => isset($items[$delta]['value']) ? $items[$delta]['value'] : NULL,
      '#placeholder' => $this->getSetting('placeholder'),
    );
    return $element;
  }

}
