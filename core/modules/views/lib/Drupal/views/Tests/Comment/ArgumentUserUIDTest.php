<?php

/**
 * @file
 * Definition of Drupal\views\Tests\Comment\ArgumentUserUIDTest.
 */

namespace Drupal\views\Tests\Comment;

/**
 * Tests the argument_comment_user_uid handler.
 */
class ArgumentUserUIDTest extends CommentTestBase {

  /**
   * Views used by this test.
   *
   * @var array
   */
  public static $testViews = array('test_comment_user_uid');

  public static function getInfo() {
    return array(
      'name' => 'Comment: User UID Argument',
      'description' => 'Tests the user posted or commented argument handler.',
      'group' => 'Views Modules',
    );
  }

  function testCommentUserUIDTest() {
    $view = views_get_view('test_comment_user_uid');
    $this->executeView($view, array($this->account->uid));
    $result_set = array(
      array(
        'nid' => $this->node_user_posted->nid,
      ),
      array(
        'nid' => $this->node_user_commented->nid,
      ),
    );
    $column_map = array('nid' => 'nid');
    $this->assertIdenticalResultset($view, $result_set, $column_map);
  }

}
