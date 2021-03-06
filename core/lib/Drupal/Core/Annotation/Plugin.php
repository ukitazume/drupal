<?php

/**
 * @file
 * Definition of Drupal\Core\Annotation\Plugin.
 */

namespace Drupal\Core\Annotation;

use Drupal\Core\Annotation\AnnotationInterface;

/**
 * Defines a Plugin annotation object.
 *
 * Annotations in plugin classes can utilize this class in order to pass
 * various metadata about the plugin through the parser to
 * DiscoveryInterface::getDefinitions() calls. This allows the metadata
 * of a class to be located with the class itself, rather than in module-based
 * info hooks.
 *
 * @Annotation
 */
class Plugin implements AnnotationInterface {

  /**
   * The plugin definiton read from the class annotation.
   *
   * @var array
   */
  protected $definition;

  /**
   * Constructs a Plugin object.
   *
   * Builds up the plugin definition and invokes the get() method for any
   * classed annotations that were used.
   */
  public function __construct($values) {
    $this->definition = $this->parse($values);
  }

  /**
   * Parses an annotation into its definition.
   *
   * @param array $values
   *   The annotation array.
   *
   * @return array
   *  The parsed annotation as a definition.
   */
  protected function parse(array $values) {
    $definitions = array();
    foreach ($values as $key => $value) {
      if ($value instanceof AnnotationInterface) {
        $definitions[$key] = $value->get();
      }
      elseif (is_array($value)) {
        $definitions[$key] = $this->parse($value);
      }
      else {
        $definitions[$key] = $value;
      }
    }
    return $definitions;
  }

  /**
   * Implements Drupal\Core\Annotation\AnnotationInterface::get().
   */
  public function get() {
    return $this->definition;
  }

}
