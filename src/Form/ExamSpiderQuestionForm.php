<?php

namespace Drupal\exam_spider\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\exam_spider\Controller\ExamSpider;

/**
 * Form builder for the add/edit Question form.
 *
 * @package Drupal\exam_spider\Form
 */
class ExamSpiderQuestionForm extends FormBase {

  /**
   * Add/Update get Question form.
   */
  public function getFormId() {
    return 'add_edit_question_form';
  }

  /**
   * Add/edit Question form.
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $examspider_service = new ExamSpider();
    $form = $exam_options = $values = $answer = [];
    $form['#attached']['library'][] = 'exam_spider/exam_spider';
    $current_path = \Drupal::service('path.current')->getPath();
    $path_args = explode('/', $current_path);
    $default_sel = $path_args[5];
    if ($path_args[6] == 'edit' && is_numeric($path_args[5])) {
      $question_id = $path_args[5];
      $values = $examspider_service->examSpiderGetQuestion($question_id);
      $answer = array_flip(explode('-', $values['answer']));
      $form['question_id'] = ['#type' => 'value', '#value' => $question_id];
      $default_sel = $values['examid'];
    }
    $all_exam = $examspider_service->examSpiderGetExam();
    foreach ($all_exam as $option) {
      $exam_options[$option->id] = $option->exam_name;
    }
    $form['#attributes'] = ['class' => ['questions-action']];
    $form['selected_exam'] = [
      '#type' => 'select',
      '#title' => $this->t('Select @examSpiderExamTitle', ['@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE]),
      '#options' => $exam_options,
      '#default_value' => isset($default_sel) ? $default_sel : NULL,
      '#required' => TRUE,
    ];
    $form['question_name'] = [
      '#title' => $this->t('Question Name'),
      '#type' => 'textfield',
      '#maxlength' => '170',
      '#required' => TRUE,
      '#default_value' => isset($values['question']) ? $values['question'] : NULL,
    ];
    $form['options'] = [
      '#type' => 'details',
      '#title' => $this->t('Option settings'),
      '#open' => TRUE,
    ];
    $form['options']['multi_answer'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Enable Multiple Answers'),
      '#default_value' => isset($values['multiple']) ? $values['multiple'] : NULL,
    ];
    for ($i = 1; $i <= 4; $i++) {

      $form['options']['opt' . $i] = [
        '#title' => $this->t('Option @i', ['@i' => $i]),
        '#type' => 'textarea',
        '#maxlength' => '550',
        '#cols' => 20,
        '#rows' => 1,
        '#required' => TRUE,
        '#default_value' => isset($values['opt' . $i]) ? $values['opt' . $i] : NULL,
        '#prefix' => '<div class="option_set">',
      ];
      $form['options']['answer' . $i] = [
        '#type' => 'checkbox',
        '#title' => $this->t('Correct Option'),
        '#attributes' => ['class' => ['answer']],
        '#default_value' => isset($answer['opt' . $i]) ? 1 : NULL,
        '#suffix' => '</div>',
      ];
    }

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Submit'),
    ];
    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Submit'),
    ];
    $exam_spider_get_questions = $examspider_service->examSpiderGetQuestionsList($default_sel);
    $form['#suffix'] = drupal_render($exam_spider_get_questions);
    return $form;
  }

  /**
   * Add/Update exam page validate callbacks.
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    $answer1 = $form_state->getValue('answer1');
    $answer2 = $form_state->getValue('answer2');
    $answer3 = $form_state->getValue('answer3');
    $answer4 = $form_state->getValue('answer4');
    if ($answer1 == 0 && $answer2 == 0 && $answer3 == 0 && $answer4 == 0) {
      return $form_state->setErrorByName('answer', $this->t('Please choose at least one answer.'));
    }
  }

  /**
   * Exam Add/Update form submit callbacks.
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $answer = '';
    for ($i = 1; $i <= 4; $i++) {
      if ($form_state->getValue('answer' . $i) == 1) {
        $answer .= 'opt' . $i . '-';
      }
    }
    $answer = rtrim($answer, "-");
    $examid = $form_state->getValue('selected_exam');
    $values['examid'] = $examid;
    $values['question'] = $form_state->getValue('question_name');
    $values['opt1'] = $form_state->getValue('opt1');
    $values['opt2'] = $form_state->getValue('opt2');
    $values['opt3'] = $form_state->getValue('opt3');
    $values['opt4'] = $form_state->getValue('opt4');
    $values['answer'] = $answer;
    $values['multiple'] = $form_state->getValue('multi_answer');
    $values['created'] = REQUEST_TIME;
    $values['changed'] = REQUEST_TIME;

    $question_id = $form_state->getValue('question_id');
    if ($question_id) {
      db_update('exam_questions')
        ->fields($values)
        ->condition('id', $question_id)
        ->execute();
      drupal_set_message($this->t('You have successfully updated question.'));
    } 
    else {
      db_insert('exam_questions')
        ->fields($values)
        ->execute();
      drupal_set_message($this->t('You have successfully created question for this @examSpiderExamTitle', ['@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE]));
    }
    $form_state->setRedirect('exam_spider.exam_spider_add_question', ['examid' => $examid]);
  }

}
