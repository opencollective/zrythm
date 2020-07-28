;;;
;;; Copyright (C) 2020 Alexandros Theodotou <alex at zrythm dot org>
;;;
;;; This file is part of Zrythm.
;;;
;;; Zrythm is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU Affero General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or (at
;;; your option) any later version.
;;;
;;; Zrythm is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU Affero General Public License for more details.
;;;
;;; You should have received a copy of the GNU Affero General Public License
;;; along with Zrythm.  If not, see <http://www.gnu.org/licenses/>.

;;; This file generates the gsettings schema

(add-to-load-path "@SCRIPTS_DIR@")

(define-module (gschema-gen)
  #:use-module (guile-utils)
  #:use-module (ice-9 string-fun)
  #:use-module (ice-9 format)
  #:use-module (ice-9 match)
  #:use-module (srfi srfi-1)
  #:use-module (srfi srfi-9) ; records
  #:use-module (sxml simple))

(define top-id "org.zrythm.Zrythm")

(define-record-type <schema-key>
  (make-schema-key name type default summary
                   description)
  schema-key?
  (name         schema-key-name)
  (type         schema-key-type)
  (default      schema-key-default)
  (summary      schema-key-summary)
  (description  schema-key-description)
  (enum         schema-key-enum
                set-schema-key-enum!)
  (range-min    schema-key-range-min
                set-schema-key-range-min!)
  (range-max    schema-key-range-max
                set-schema-key-range-max!))

(define (make-schema-key-with-enum
          name enum default summary description)
  (let ((key (make-schema-key
               name "" default summary description)))
    (set-schema-key-enum! key enum)
    key))

(define (make-schema-key-with-range
          name type range-min range-max default
          summary description)
  (let ((key (make-schema-key
               name type default summary description)))
    (set-schema-key-range-min! key range-min)
    (set-schema-key-range-max! key range-max)
    key))

(define-record-type <schema>
  (make-schema local-id keys)
  schema?
  ;; ID after "org.zrythm.Zrythm."
  (local-id schema-local-id)
  (keys     schema-keys)
  (preferences-category
    schema-preferences-category
    set-schema-preferences-category!))

(define (schema-id schema)
  (string-append
    top-id "."
    (if (string?
            (schema-preferences-category schema))
      (string-append
        "preferences."
        (schema-preferences-category schema) ".")
      "")
    (schema-local-id schema)))

(define (schema-path schema)
  (string-append
    "/"
    (string-replace-substring
      (schema-id schema) "." "/")
    "/"))

(define (schema-print schema)
  ;; display header
  (format #t "  <schema id=~s
        path=~s>\n"
        (schema-id schema)
        (schema-path schema))
  ;; display keys
  (fold
    (lambda (key keys)
      (format #t
        "    <key name=~s ~a=~s>~a
      <default>~a</default>
      <summary>~a</summary>
      <description>~a</description>
    </key>\n"
        (schema-key-name key)
        (if (string? (schema-key-enum key))
          "enum"
          "type")
        (if (string? (schema-key-enum key))
          (string-append
            top-id "." (schema-key-enum key) "-enum")
          (schema-key-type key))
        (if (string? (schema-key-range-min key))
          (format #f "\n      <range min=~s max=~s />"
                  (schema-key-range-min key)
                  (schema-key-range-max key))
          "")
        (if (or
              (string? (schema-key-enum key))
              (string=? (schema-key-type key) "s"))
          (format #f "~s"
                  (schema-key-default key))
          (schema-key-default key))
        (schema-key-summary key)
        (schema-key-description key)))
    '()
    (schema-keys schema))
  ;; close
  (display "  </schema>\n\n"))

;; leave for reference
(define (print-enum-using-sxml in-id nicks)
  (display "  ")
  (let* ((enum-id
           (string-append top-id "."
                          in-id "-enum"))
         (nicks
           (let loop ((nicks nicks)
                      (idx 0)
                      (res '()))
             (match nicks
               (() (reverse res))
               ((nick . rest)
                (loop
                  rest (1+ idx)
                  (cons `(value (@ (nick ,nick)
                                   (value ,idx)))
                        res))))))
         (sxml `(enum (@ (id ,enum-id)) ,nicks)))
    (display (sxml->xml sxml)))
  (newline))

(define (print-enum id nicks)
  (display
    (string-append
      "  <enum id=\"" top-id "." id "-enum\">\n"))
  (fold
    (lambda (nick idx)
      (format #t
        "    <value nick=\"~a\" value=\"~d\"/>\n"
        nick idx)
      (1+ idx))
    0
    nicks)
  (display "  </enum>\n"))

(define-record-type <preferences-category>
  (make-preferences-category name schemas)
  preferences-category?
  (name         preferences-category-name)
  (schemas      preferences-category-schemas))

(define (preferences-category-print category)
  ;; display each schema in this preference category
  (fold
    (lambda (schema schemas)
      (set-schema-preferences-category!
        schema (preferences-category-name category))
      (schema-print schema))
    '()
    (preferences-category-schemas category)))

#!
Args:
1: output file
2: top schema delimited by "."
!#
(define (main . args)

  ;; verify number of args
  (unless (eq? (length args) 3)
    (display "Need 2 arguments")
    (newline)
    (exit -1))

  ;; get args
  (match args
    ((this-program output-file in-top-id)

     (set! top-id in-top-id)

     ;; open file
     (with-output-to-file output-file
       (lambda ()

         ;; print top
         (display
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<!--

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

<schemalist
  gettext-domain=\"zrythm\">

")

         ;; print enums
         (print-enum
           "audio-backend"
           '("none" "alsa" "jack" "portaudio" "sdl"
             "rtaudio"))
         (print-enum
           "midi-backend"
           '("none" "alsa" "jack" "windows-mme"
             "rtmidi"))
         (print-enum
           "language"
           '("ar" "cs" "da" "de" "en" "en_GB" "el"
             "es" "et" "fi" "fr" "gd" "gl" "hi" "it"
             "ja" "ko" "nb_NO" "nl" "pl" "pt" "pt_BR"
             "ru" "sv" "zh"))
         (print-enum
           "export-time-range"
           '("loop" "song" "custom"))
         (print-enum
           "export-format"
           '("flac" "ogg" "wav" "mp3" "mid"))
         (print-enum
           "export-bit-depth"
           '("16" "24"))
         ;; TODO
         (print-enum
           "note-length"
           '("2/1" "1/1" "1/2"))
         (print-enum
           "note-type"
           '("normal" "dotted" "triplet"))
         (print-enum
           "midi-modifier"
           '("velocity" "aftertouch"))
         (print-enum
           "note-notation"
           '("notes" "numbers"))
         (print-enum
           "plugin-browser-tab"
           '("collection" "category" "protocol"))
         (print-enum
           "plugin-browser-filter"
           '("none" "instrument" "effect" "modulator"
             "midi-effect"))
         (print-enum
           "piano-roll-highlight"
           '("none" "chord" "scale" "both"))
         (print-enum
           "pan-law"
           '("zero-db" "minus-three-db"
             "minus-six-db" ))
         (print-enum
           "pan-algorithm"
           '("linear" "sqrt" "sine"))
         (print-enum
           "curve-algorithm"
           '("exponent" "superellipse" "vital"))
         (print-enum
           "buffer-size"
           '("16" "32" "64" "128" "256" "512" "1024"
             "2048" "4096"))
         (print-enum
           "sample-rate"
           '("22050" "32000" "44100" "48000" "88200"
             "96000" "192000"))
         (print-enum
           "transport-display"
           '("bbt" "time"))
         (print-enum
           "tool"
           '("select-normal" "select-stretch"
             "edit" "cut" "erase" "ramp"
             "audition"))
         (print-enum
           "graphic-detail"
           '("high" "normal" "low"))
         (newline)

         ;; -- print normal schemas --

         (schema-print
           (make-schema
             "general"
             (list
               (make-schema-key
                 "recent-projects" "as" "[]"
                 "Recent project list"
                 "A list of recent projects to be referenced on startup.")
               (make-schema-key
                 "first-run" "b" "true"
                 "First run"
                 "Whether this is the first run or not.")
               (make-schema-key
                 "install-dir" "s" ""
                 "Installation directory"
                 "This is the directory Zrythm is installed in. Currently only used on Windows.")
               (make-schema-key
                 "last-project-dir" "s" ""
                 "Last project directory"
                 "Last directory a project was created in.")
             ))) ;; general

         (schema-print
           (make-schema
             "ui"
             (list
               (make-schema-key
                 "bounce-with-effects" "b" "true"
                 "Bounce with effects"
                 "If set to true, the effects chain will be included in the bounced material.")
               (make-schema-key
                 "bounce-tail" "i" "0"
                 "Bounce tail"
                 "Tail to allow when bouncing (for example to catch reverb tails), in milliseconds.")
               (make-schema-key-with-enum
                 "timeline-object-length"
                 "note-length" "1/1"
                 "Timeline object length"
                 "Default length to use when creating timelinle objects, eg in edit mode.")
               (make-schema-key-with-enum
                 "timeline-object-length-type"
                 "note-type" "normal"
                 "Timeline object length type"
                 "Default length type to use when creating timelinle objects, eg in edit mode.")
               (make-schema-key-with-enum
                 "editor-object-length"
                 "note-length" "1/1"
                 "Editor object length"
                 "Default length to use when creating timelinle objects, eg in edit mode.")
               (make-schema-key-with-enum
                 "editor-object-length-type"
                 "note-type" "normal"
                 "Editor object length type"
                 "Default length type to use when creating timelinle objects, eg in edit mode.")
               (make-schema-key-with-enum
                 "piano-roll-note-notation"
                 "note-notation" "notes"
                 "Note notation"
                 "The note notation used in the piano roll - MIDI pitch index or notes (C, C#, etc.)")
               (make-schema-key
                 "musical-mode" "b" "true"
                 "Musical mode"
                 "Whether to use musical mode. If this is on, time-stretching will be applied to events so that they match the project BPM. This mostly applies to audio regions.")
               (make-schema-key
                 "listen-notes" "b" "true"
                 "Listen to notes while they are moved"
                 "Whether to listen to MIDI notes while dragging them in the piano roll.")
               (make-schema-key-with-enum
                 "piano-roll-highlight"
                 "piano-roll-highlight" "none"
                 "Piano roll highlight"
                 "Whether to highlight chords, scales, both or none in the piano roll.")
               (make-schema-key-with-enum
                 "piano-roll-midi-modifier"
                 "midi-modifier" "velocity"
                 "Piano roll MIDI modifier"
                 "The MIDI modifier to display in the MIDI editor (only velocity is valid at the moment).")
               (make-schema-key
                 "browser-divider-position" "i" "220"
                 "Browser divider position"
                 "Height of the top part of the plugin/file browser.")
               (make-schema-key
                 "left-panel-divider-position" "i"
                 "180"
                 "Left panel divider position"
                 "Position of the resize handle of the left panel.")
               (make-schema-key
                 "right-panel-divider-position" "i"
                 "180"
                 "Right panel divider position"
                 "Position of the resize handle of the right panel.")
               (make-schema-key
                 "bot-panel-divider-position" "i"
                 "180"
                 "Bot panel divider position"
                 "Position of the resize handle of the bot panel.")
               (make-schema-key-with-enum
                 "plugin-browser-tab"
                 "plugin-browser-tab" "category"
                 "Plugin browser tab"
                 "Selected plugin browser tab.")
               (make-schema-key-with-enum
                 "plugin-browser-filter"
                 "plugin-browser-filter" "none"
                 "Plugin browser filter"
                 "Selected plugin browser filter")
               (make-schema-key
                 "timeline-event-viewer-visible" "b"
                 "false"
                 "Timeline event viewer visibility"
                 "Whether the timeline event viewer is visible or not.")
               (make-schema-key
                 "editor-event-viewer-visible" "b"
                 "false"
                 "Editor event viewer visibility"
                 "Whether the editor event viewer is visible or not.")
               (make-schema-key-with-range
                 "monitor-out-vol" "d"
                 "0.0" "2.0" "1.0"
                 "Monitor out volume"
                 "The monitor out volume in amplitude (0 to 2).")
               (make-schema-key
                 "plugin-favorites" "as"
                 "[ \"My Synths::ZSaw::ZSaw-trial\" ]"
                 "Plugin favorites"
                 "A list of plugin favorites as strings separated by '::', where the first element is the name of the list.")
               (make-schema-key-with-enum
                 "transport-display"
                 "transport-display" "bbt"
                 "Transport display type"
                 "Selected transport display type (BBT/time).")
               (make-schema-key-with-enum
                 "selected-tool"
                 "tool" "select-normal"
                 "Selected editing tool"
                 "Selected editing tool.")
             ))) ;; ui

         (schema-print
           (make-schema
             "export"
             (list
               (make-schema-key
                 "genre" "s" "Electronic"
                 "Genre"
                 "Genre to use when exporting, if the file type supports it.")
               (make-schema-key
                 "artist" "s" "Zrythm"
                 "Artist"
                 "Artist to use when exporting, if the file type supports it.")
               (make-schema-key-with-enum
                 "time-range" "export-time-range"
                 "song"
                 "Time range"
                 "Time range to export.")
               (make-schema-key-with-enum
                 "format" "export-format" "flac"
                 "Format"
                 "Format to export to.")
               (make-schema-key
                 "dither" "b" "false"
                 "Dither"
                 "Add dither while exporting, if applicable.")
               (make-schema-key-with-enum
                 "bit-depth"
                 "export-bit-depth" "24"
                 "Bit depth"
                 "Bit depth to use when exporting")
             ))) ;; export

         (schema-print
           (make-schema
             "ui.inspector"
             (list
               (make-schema-key
                 "track-properties-expanded" "b"
                 "true"
                 "Track properties expanded"
                 "Whether track properties is expanded.")
               (make-schema-key
                 "track-outputs-expanded" "b"
                 "true"
                 "Track outputs expanded"
                 "Whether track outputs is expanded.")
               (make-schema-key
                 "track-sends-expanded" "b"
                 "true"
                 "Track sends expanded"
                 "Whether track sends is expanded.")
               (make-schema-key
                 "track-inputs-expanded" "b"
                 "true"
                 "Track inputs expanded"
                 "Whether track inputs is expanded.")
               (make-schema-key
                 "track-controls-expanded" "b"
                 "true"
                 "Track controls expanded"
                 "Whether track controls is expanded.")
               (make-schema-key
                 "track-inserts-expanded" "b"
                 "true"
                 "Track inserts expanded"
                 "Whether track inserts is expanded.")
               (make-schema-key
                 "track-midi-fx-expanded" "b"
                 "true"
                 "Track midi-fx expanded"
                 "Whether track midi-fx is expanded.")
               (make-schema-key
                 "track-fader-expanded" "b"
                 "true"
                 "Track fader expanded"
                 "Whether track fader is expanded.")
               (make-schema-key
                 "track-comment-expanded" "b"
                 "true"
                 "Track comment expanded"
                 "Whether track comment is expanded.")
             ))) ;; ui.inspector

         (schema-print
           (make-schema
             "transport"
             (list
               (make-schema-key
                 "loop" "b"
                 "true"
                 "Transport loop"
                 "Whether looping is enabled.")
               (make-schema-key
                 "return-to-cue" "b"
                 "true"
                 "Return to cue"
                 "Whether return to cue on stop is enabled.")
               (make-schema-key
                 "metronome-enabled" "b" "false"
                 "Metronome enabled"
                 "Whether the metronome is enabled.")
             ))) ;; transport

         ;; -- print preferences schemas --
         ;; the first key in each schema should
         ;; be called "info" and have the group in
         ;; the summary and the subgroup in the
         ;; description
         ;; the value is [
         ;;   sort index of group,
         ;;   sort index of child ]

         (preferences-category-print
           (make-preferences-category
             "general"
             (list
               (make-schema
                 "engine"
                 (list
                   (make-schema-key
                     "info" "ai" "[0,0]"
                     "General" "Engine")
                   (make-schema-key-with-enum
                     "audio-backend" "audio-backend"
                     "none" "Audio backend"
                     "The audio backend to use.")
                   (make-schema-key
                     "rtaudio-audio-device-name" "s"
                     "" "RtAudio device"
                     "The name of the RtAudio device to use.")
                   (make-schema-key
                     "sdl-audio-device-name" "s"
                     "" "SDL device"
                     "The name of the SDL device to use.")
                   (make-schema-key-with-enum
                     "sample-rate" "sample-rate"
                     "48000" "Samplerate"
                     "Samplerate to pass to the backend.")
                   (make-schema-key-with-enum
                     "buffer-size" "buffer-size"
                     "512" "Buffer size"
                     "Buffer size to pass to the backend.")
                   (make-schema-key-with-enum
                     "midi-backend" "midi-backend"
                     "none" "MIDI backend"
                     "The MIDI backend to use.")
                   (make-schema-key
                     "midi-controllers" "as"
                     "[]" "MIDI controllers"
                     "A list of controllers to auto-connect to.")
                 )) ;; general/engine
               (make-schema
                 "paths"
                 (list
                   (make-schema-key
                     "info" "ai" "[0,1]"
                     "General" "Paths")
                   (make-schema-key
                     "zrythm-dir" "s"
                     "" "Zrythm path"
                     "The directory used to save user data in.")
                 )) ;; general/paths
             ))) ;; general

         (preferences-category-print
           (make-preferences-category
             "plugins"
             (list
               (make-schema
                 "uis"
                 (list
                   (make-schema-key
                     "info" "ai" "[1,0]"
                     "Plugins" "UIs")
                   (make-schema-key
                     "open-on-instantiate" "b"
                     "true" "Open UI on instantiation"
                     "Open plugin UIs when they are instantiated.")
                   (make-schema-key
                     "stay-on-top" "b"
                     "true" "Keep window on top"
                     "Always show plugin UIs on top of the main window.")
                   (make-schema-key
                     "generic" "b"
                     "false" "Generic UIs"
                     "Show generic plugin UIs generated by Zrythm instead of custom ones.")
                   (make-schema-key
                     "bridge-unsupported" "b"
                     "true" "Bridge unsupported UIs"
                     "Bridge unsupported UIs in another process instead of creating generic ones. This will lead to some performance loss.")
                   (make-schema-key-with-range
                     "refresh-rate" "i" "0" "180"
                     "0" "Refresh rate"
                     "Refresh rate in Hz. If set to 0, the screen's refresh rate will be used.")
                 )) ;; plugins/uis
               (make-schema
                 "paths"
                 (list
                   (make-schema-key
                     "info" "ai" "[1,1]"
                     "Plugins" "Paths")
                   (make-schema-key
                     "vst-search-paths-windows" "as"
                     "[ \"C:\\\\Program Files\\\\Common Files\\\\VST2\",
        \"C:\\\\Program Files\\\\VSTPlugins\",
        \"C:\\\\Program Files\\\\Steinberg\\\\VSTPlugins\",
        \"C:\\\\Program Files\\\\Common Files\\\\VST2\",
        \"C:\\\\Program Files\\\\Common Files\\\\Steinberg\\\\VST2\" ]"
                     "VST plugins"
                     "The search paths to scan for VST plugins in. Duplicate paths will be deduplicated.")
                   (make-schema-key
                     "sfz-search-paths" "as" "[]"
                     "SFZ instruments"
                     "The search paths to scan for SFZ instruments in. Duplicate paths will be deduplicated.")
                   (make-schema-key
                     "sf2-search-paths" "as" "[]"
                     "SF2 instruments"
                     "The search paths to scan for SF2 instruments in. Duplicate paths will be deduplicated.")
                 )) ;; plugins/paths
             ))) ;; plugins

         (preferences-category-print
           (make-preferences-category
             "editing"
             (list
               (make-schema
                 "audio"
                 (list
                   (make-schema-key
                     "info" "ai" "[3,0]"
                     "Editing" "Audio")
                   (make-schema-key-with-enum
                     "fade-algorithm" "curve-algorithm"
                     "superellipse" "Fade algorithm"
                     "Default fade algorithm to use for fade in/outs.")
                 )) ;; editing/audio
               (make-schema
                 "automation"
                 (list
                   (make-schema-key
                     "info" "ai" "[3,1]"
                     "Editing" "Automation")
                   (make-schema-key-with-enum
                     "curve-algorithm"
                     "curve-algorithm"
                     "superellipse"
                     "Curve algorithm"
                     "Default algorithm to use for automation curves.")
                 )) ;; editing/automation
               (make-schema
                 "undo"
                 (list
                   (make-schema-key
                     "info" "ai" "[3,2]"
                     "Editing" "Undo")
                   (make-schema-key-with-range
                     "undo-stack-length" "i" "-1"
                     "380000" "128"
                     "Undo stack length"
                     "Maximum undo history stack length. Set to -1 for unlimited.")
                 )) ;; editing/undo
             ))) ;; editing

         (preferences-category-print
           (make-preferences-category
             "projects"
             (list
               (make-schema
                 "general"
                 (list
                   (make-schema-key
                     "info" "ai" "[4,0]"
                     "Projects" "General")
                   (make-schema-key-with-range
                     "autosave-interval" "u"
                     "0" "120" "1"
                     "Autosave interval"
                     "Interval to auto-save projects, in minutes. Auto-saving will be disabled if this is set to 0.")
                 )) ;; projects/general
             ))) ;; projects

         (preferences-category-print
           (make-preferences-category
             "ui"
             (list
               (make-schema
                 "general"
                 (list
                   (make-schema-key
                     "info" "ai" "[5,0]"
                     "UI" "General")
                   (make-schema-key-with-enum
                     "graphic-detail"
                     "graphic-detail" "high"
                     "Draw detail"
                     "Level of detail to use when drawing graphics.")
                   (make-schema-key-with-enum
                     "language" "language"
                     "en"
                     "User interface language"
                     "The language to use for the user interface.")
                 )) ;; ui/general
             ))) ;; ui

         (preferences-category-print
           (make-preferences-category
             "dsp"
             (list
               (make-schema
                 "pan"
                 (list
                   (make-schema-key
                     "info" "ai" "[2,0]"
                     "DSP" "Pan")
                   (make-schema-key-with-enum
                     "pan-algorithm" "pan-algorithm"
                     "sine"
                     "Pan algorithm"
                     "The panning algorithm to use when applying pan on mono signals (not used at the moment).")
                   (make-schema-key-with-enum
                     "pan-law" "pan-law"
                     "minus-three-db"
                     "Pan law"
                     "The pan law to use when applying pan on mono signals (not used at the moment).")
                 )) ;; dsp/pan
             ))) ;; dsp

         (display "</schemalist>"))))))

(apply main (program-arguments))
