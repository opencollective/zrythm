;;;  Copyright (C) 2018-2020 Alexandros Theodotou <alex at zrythm dot org>
;;;
;;;  This file is part of Zrythm
;;;
;;;  Zrythm is free software: you can redistribute it and/or modify
;;;  it under the terms of the GNU Affero General Public License as published by
;;;  the Free Software Foundation, either version 3 of the License, or
;;;  (at your option) any later version.
;;;
;;;  Zrythm is distributed in the hope that it will be useful,
;;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;;  GNU Affero General Public License for more details.
;;;
;;;  You should have received a copy of the GNU Affero General Public License
;;;  along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
;;;
;;; Generate gtk.gresources.xml
;;;
;;; Usage: gen-gtk-gresources-xml SRCDIR_GTK [OUTPUT-FILE]

(add-to-load-path "@SCRIPTS_DIR@")

(define-module (gen-gtk-resources-xml)
  #:use-module (guile-utils)
  #:use-module (ice-9 format)
  #:use-module (ice-9 match)
  #:use-module (ice-9 ftw))

(define (remove-prefix str prefix)
  (if (string-prefix? prefix str)
    (string-drop str (string-length prefix))
    str))

#!
Args:
1: resources dir
2: output file
!#
(define (main . args)

  ;; verify number of args
  (unless (eq? (length args) 3)
    (display "Need 2 arguments")
    (newline)
    (exit -1))

  ;; get args
  (match args
    ((this-program resources-dir output-file)

     ;; open file
     (with-output-to-file output-file
       (lambda ()

         (display
"<!--
  Copyright (C) 2018-2020 Alexandros Theodotou <alex at zrythm dot org>

  This file is part of Zrythm

  Zrythm is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Zrythm is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
-->
<?xml version='1.0' encoding='UTF-8'?>
<gresources>
  <gresource prefix='/org/gtk/libgtk'>
    <file>theme/Matcha-dark-sea/gtk.css</file>
    <file>theme/Matcha-dark-sea/gtk-dark.css</file>
")

         ;; print matcha image assets
         (let ((dark-sea-assets-dir
                 "theme/Matcha-dark-sea/assets"))
           (for-each
             (lambda (x)
               (display
                 (string-append
                   "    <file>"
                   dark-sea-assets-dir "/" x
                   "</file>"))
               (newline))
             (scandir
               (join-path
                 (list resources-dir
                       dark-sea-assets-dir))
               (lambda (f)
                 (or
                   (string-suffix? ".png" f)
                   (string-suffix? ".svg" f))))))

         (display
"  </gresource>
  <gresource prefix='/org/zrythm/Zrythm/app'>
")

         ;; Print UI files
         (for-each
           (lambda (x)
             (display
               (string-append
                 "    <file preprocess="
                 "'xml-stripblanks'>ui/" x
                 "</file>"))
             (newline))
           (scandir
             (join-path
               (list resources-dir "ui"))
             (lambda (f)
               (string-suffix? ".ui" f))))

         ;; add icons except breeze
         (for-each
           (lambda (dir)
             (for-each
               (lambda (icon-file)
                 (display
                   (string-append
                     "    <file alias=\"icons/"
                     dir "/" dir "-" icon-file
                     "\">icons/"
                     dir "/" icon-file
                     "</file>"))
                 (newline)
                 (display
                   (string-append
                     "    <file>icons/"
                     dir "/" icon-file
                     "</file>"))
                 (newline))
               (scandir
                 (join-path
                   (list resources-dir "icons" dir))
                 (lambda (f)
                   (or
                     (string-suffix? ".svg" f)
                     (string-suffix? ".png" f))))))
           '("gnome-builder" "ext"
             "fork-awesome" "font-awesome"))

         ;; add theme and close
         (display
"  </gresource>
</gresources>"))))))

(apply main (program-arguments))
