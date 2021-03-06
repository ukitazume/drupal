<?php

/**
 * @file
 * Definition of Drupal\file\Plugin\views\field\File.
 */

namespace Drupal\file\Plugin\views\field;

use Drupal\views\ViewExecutable;
use Drupal\Core\Annotation\Plugin;
use Drupal\views\Plugin\views\field\FieldPluginBase;

/**
 * Field handler to provide simple renderer that allows linking to a file.
 *
 * @ingroup views_field_handlers
 *
 * @Plugin(
 *   id = "file",
 *   module = "file"
 * )
 */
class File extends FieldPluginBase {

  /**
   * Constructor to provide additional field to add.
   */
  public function init(ViewExecutable $view, &$options) {
    parent::init($view, $options);
    if (!empty($options['link_to_file'])) {
      $this->additional_fields['uri'] = 'uri';
    }
  }

  protected function defineOptions() {
    $options = parent::defineOptions();
    $options['link_to_file'] = array('default' => FALSE, 'bool' => TRUE);
    return $options;
  }

  /**
   * Provide link to file option
   */
  public function buildOptionsForm(&$form, &$form_state) {
    $form['link_to_file'] = array(
      '#title' => t('Link this field to download the file'),
      '#description' => t("Enable to override this field's links."),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_file']),
    );
    parent::buildOptionsForm($form, $form_state);
  }

  /**
   * Render whatever the data is as a link to the file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_file']) && $data !== NULL && $data !== '') {
      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = file_create_url($this->get_value($values, 'uri'));
    }

    return $data;
  }

  function render($values) {
    $value = $this->get_value($values);
    return $this->render_link($this->sanitizeValue($value), $values);
  }

}
