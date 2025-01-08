(*
  Aymene Yacine Laref
  LARA10049907
*)

module Utils = Utils
open Utils

module Atom = struct
  type command =
    | ForwardDraw of string  (* F ou Funstringarbitraire *)
    | Forward of string   (* f ou funstringarbitraire *)
    | TurnLeft     (* - *)
    | TurnRight   (* + *)
    | SaveState   (* [ *)
    | GetState   (* ] *)
    | Replacable of string
    
  type t = {
    commands: command list;
  }

  (* Converti une commande en string *)
  let to_string_command (c: command) : string =
    match c with
    | ForwardDraw s -> s
    | Forward s -> s
    | TurnLeft -> "-"
    | TurnRight -> "+"
    | SaveState -> "["
    | GetState -> "]"
    | Replacable s -> s

  (* Converti un atom en string *)
  let to_string (atom: t) : string =
    let command_strings = List.map to_string_command atom.commands in
    String.concat " " command_strings
end

module System = struct
  (* On fix la distance d'avancement *)
  let forward_distance : float = -10.0

  (* On definie un etat comme un coord et un angle *)
  type state = {
    pos: Coord.t;
    angle: float;
  }

  (* Notre memoire est une pile d'etat *)
  type memory = state list

  (* Une regle est compose d'une regle et de son remplacement *)
  type rule = {
    from: string;
    too: string;
  }

  type t = {
    iterations: int;          (* Le nombre d'iterations *)
    turn_angle: float;        (* L'angle de rotation au depart *)
    rules: rule list;         (* On a une liste de regle *)
    cursor: state;            (* Chaque systeme a un etat courant *)
    memory_stack: memory;     (* Chaque systeme a une pile memoire *)
    draw_list: Line.t list;   (* Liste de line a dessiner *)
    atom_expr: string;        (* Le *)
  }

  (* Pousse un etat sur la pile de memoire *)
  let push_state_mem (ms: memory) (ns: state) : memory = ns :: ms

  (* Pop un etat de la pile de memoire *)
  let pop_state_mem (ms: memory) : (state * memory) =
    match ms with
    | [] -> raise (Failure "Memory stack is empty")
    | s :: rest -> (s, rest)

  (* Ajoute une ligne a la liste des lignes a dessiner *)
  let add_draw (dl: Line.t list) (l: Line.t): Line.t list =
    l :: dl

  (* Avance le curseur vers l'avant *)
  let move_forward (current: state) : state =
    let next_pos = {
      Coord.x = forward_distance *. sin(current.angle) +. current.pos.Coord.x;
      Coord.y = forward_distance *. cos(current.angle) +. current.pos.Coord.y;
    } in
    let next_cursor = {
      pos = next_pos;
      angle = current.angle;
    } in
    next_cursor

  (* Obtient une ligne entre deux etats *)
  let get_line (s1: state) (s2: state) : Line.t =
    let l = {
      Line.origin = s1.pos;
      destination = s2.pos;
    } in l

  (* Tourne le curseur *)
  let turn (cursor: state) (angle: float) (clockwise: bool) : state =
    let two_pi = 2.0 *. Float.pi in
    let new_angle =
      if clockwise then
        mod_float (cursor.angle -. angle) two_pi
      else
        mod_float (cursor.angle +. angle) two_pi
    in
    { cursor with angle = new_angle }

  exception InvalidSystem

  (* Split une string par un delimitateur *)
  let split_string (s: string) (delimiter: string) : string list =
    Str.split (Str.regexp_string delimiter) s

  (* Permet de passer d'un string a un atom *)
  let atom_from_string (s: string) : Atom.t =
    let words = Str.split (Str.regexp_string " ") s in
    let rec aux (words: string list) (parsed : Atom.command list) : Atom.command list =
      match words with
      | [] -> List.rev parsed
      | first::rest ->
        let cmd = match first with
          | _ when String.length first > 0 && first.[0] = 'F' -> Some (Atom.ForwardDraw first)
          | _ when String.length first > 0 && first.[0] = 'f' -> Some (Atom.Forward first)
          | "-" -> Some Atom.TurnLeft
          | "+" -> Some Atom.TurnRight
          | "[" -> Some Atom.SaveState
          | "]" -> Some Atom.GetState
          | _ -> Some (Atom.Replacable first)
        in
        let parsed = match cmd with
          | Some command -> command :: parsed
          | None -> parsed
        in
        aux rest parsed
    in
    let atom_ret = { Atom.commands = aux words [] } in
    atom_ret

  (* Prend le n-ieme element d'une liste *)
  let rec nth_elem (l: 'a list) (n: int) : 'a =
    match (l, n) with
    | first::_, 0 -> first
    | _::rest, n -> nth_elem rest (n-1)
    | [], _ -> raise InvalidSystem

  (* Supprime les n premiers elements d'une liste *)
  let drop_list (n: int) (lst: 'a list) : 'a list =
    let rec aux (i: int) (lst: 'a list) : 'a list =
      match lst with
      | [] -> []
      | _::xs when i < n -> aux (i + 1) xs
      | lst -> lst
    in
    aux 0 lst

  (* Prend une sous-chaine *)
  let substring (str: string) (delimiter: string) (sens: bool) : string =
    let len = String.length str in
    let delimlen = String.length delimiter in
    let rec aux (i: int) : int =
      if i < 0 || i > len - delimlen then
        raise (Invalid_argument "Invalid definition, assignment character not found")
      else if String.sub str i delimlen = delimiter then
        i
      else 
        aux (if sens then i + 1 else i - 1)
    in
    let idx = aux (if sens then 0 else len - delimlen) in
    if sens then
      String.sub str 0 idx
    else 
      String.sub str (idx + delimlen) (len - idx - delimlen)

  (* Trim une string *)
  let trim (str: string) : string =
    let len = String.length str in
    let start = if String.get str 0 = ' ' then 1 else 0 in
    let finish = if String.get str (len - 1) = ' ' then 
                    len - 2 
                else 
                  len - 1 in
    if start > finish then ""
      else String.sub str start (finish - start + 1)    

  (* Converti une string en regle *)
  let rule_of_string (s: string) : rule =
    let from = trim (substring s ":" true) in
    let too  = trim (substring s ":" false) in
    { from; too }
    
  (* Parse les regles *)
  let rec parse_rules (r: string list) (acc: rule list) : rule list  =
    match r with
    | [] -> List.rev acc
    | first::rest -> parse_rules rest ((rule_of_string first) :: acc)

  (* Permet de creer un L-System a partir d'une string *) 
  let of_string (input : string) : t =
    try
      let split_input = split_string input "\n" in
      let iter_string = nth_elem split_input 0 in
      let turn_angle_string = nth_elem split_input 1 in
      let base_atom_string = nth_elem split_input 2 in
      let rules_string = drop_list 3 split_input in
      {
        cursor = { pos = { Coord.x = 0.0; Coord.y = 0.0 }; angle = 0.0 };
        memory_stack = [];
        draw_list = [];
        iterations = int_of_string ( trim (substring iter_string "=" false));
        turn_angle = -1. *. float_of_string ( trim (substring turn_angle_string "=" false)) *. Float.pi /. 180.;
        atom_expr = trim (substring base_atom_string "=" false);
        rules = parse_rules rules_string [];
      }
    with
    | Invalid_argument _ -> raise InvalidSystem

  (* Trouve une regle correspondante *)
  let find_matching_rule (expr: string) (idx: int) (rules: rule list) : (rule * int) option =
    let rec find (rules: rule list) : (rule * int) option =
      match rules with
      | [] -> None
      | rule :: rest ->
        let re = Str.regexp_string rule.from in
        try
          let _ = Str.search_forward re expr idx in
          if Str.match_beginning () = idx then
            Some (rule, Str.match_end ())
          else
            find rest
        with Not_found -> find rest
    in
    find rules

  (* Applique une regle correspondante *)
  let apply_matched_rule (buffer: Buffer.t) (expr: string) (idx: int) (rule: rule) (match_end: int) : int =
    let match_start = Str.match_beginning () in
    Buffer.add_substring buffer expr idx (match_start - idx);
    Buffer.add_string buffer rule.too;
    match_end

  (* Fonction recursive principale pour traverser et appliquer les regles *)
  let rec aux (buffer: Buffer.t) (expr: string) (idx: int) (rules: rule list) : string =
    if idx >= String.length expr then
      Buffer.contents buffer
    else
      match find_matching_rule expr idx rules with
      | Some (rule, match_end) ->
        let new_idx = apply_matched_rule buffer expr idx rule match_end in
        aux buffer expr new_idx rules
      | None ->
        Buffer.add_char buffer expr.[idx];
        aux buffer expr (idx + 1) rules

  (* Fonction principale pour appliquer les regles en parallele *)
  let apply_rules_parallele (expr: string) (rules: rule list) : string =
    let buffer = Buffer.create (String.length expr) in
    aux buffer expr 0 rules

  (* Fonction principale pour appliquer les regles sur plusieurs iterations *)
  let apply ?iterations (system: t) : Atom.t =
    let iterations = match iterations with
      | Some n -> n
      | None -> system.iterations
    in
    let rec aux (iterations: int) (s: string) : string =
      if iterations = 0 then
        s
      else
        let updated = apply_rules_parallele s system.rules in
        aux (iterations - 1) updated
    in
    atom_from_string (aux iterations system.atom_expr)

  (* Effet de chaque action *)
  let action (system: t) (command: Atom.command) : t =
    match command with
    | Atom.ForwardDraw _ ->
      let new_cursor = move_forward system.cursor in
      let new_line = get_line system.cursor new_cursor in
      let new_draw_list = add_draw system.draw_list new_line in
      { system with cursor = new_cursor; draw_list = new_draw_list }
    | Atom.Forward _ ->
      let new_cursor = move_forward system.cursor in
      { system with cursor = new_cursor }
    | Atom.TurnLeft ->
      let new_cursor = turn system.cursor system.turn_angle false in
      { system with cursor = new_cursor }
    | Atom.TurnRight ->
      let new_cursor = turn system.cursor system.turn_angle true in
      { system with cursor = new_cursor }
    | Atom.SaveState ->
      let new_memory_stack = push_state_mem system.memory_stack system.cursor in
      { system with memory_stack = new_memory_stack }
    | Atom.GetState ->
      let new_cursor, new_memory_stack = pop_state_mem system.memory_stack in
      { system with cursor = new_cursor; memory_stack = new_memory_stack }
    | Atom.Replacable _ -> system

  (* Interprete les commandes *)
  let interpret (system: t) (expr: Atom.command list) : t =
    List.fold_left (fun acc cmd -> action acc cmd) system expr

  (* Calcule le canvas *)
  let calculate_canvas (lines: Line.t list) : Coord.t * Coord.t =
    match lines with
    | [] -> ({ Coord.x = 0.0; Coord.y = 0.0 }, { Coord.x = 0.0; Coord.y = 0.0 })
    | initial_line :: _ ->
      let initial_min_x = min initial_line.Line.origin.Coord.x initial_line.Line.destination.Coord.x in
      let initial_max_x = max initial_line.Line.origin.Coord.x initial_line.Line.destination.Coord.x in
      let initial_min_y = min initial_line.Line.origin.Coord.y initial_line.Line.destination.Coord.y in
      let initial_max_y = max initial_line.Line.origin.Coord.y initial_line.Line.destination.Coord.y in
      let min_x, max_x =
        List.fold_left
          (fun (min_x, max_x) line ->
            let start_x = line.Line.origin.Coord.x in
            let end_x = line.Line.destination.Coord.x in
            (min min_x (min start_x end_x), max max_x (max start_x end_x)))
          (initial_min_x, initial_max_x)
          lines
      in
      let min_y, max_y =
        List.fold_left
          (fun (min_y, max_y) line ->
            let start_y = line.Line.origin.Coord.y in
            let end_y = line.Line.destination.Coord.y in
            (min min_y (min start_y end_y), max max_y (max start_y end_y)))
          (initial_min_y, initial_max_y)
          lines
      in
      ({ Coord.x = min_x; Coord.y = min_y }, { Coord.x = max_x; Coord.y = max_y })

  (* Dessine le dessein *)
  let draw ?iterations ?angle (system: t) : Canvas.t =
    let system = match angle with
      | Some a -> { system with turn_angle = a }  
      | None -> system
    in
    let final_atom = apply ?iterations system in
    let final_system = interpret system final_atom.commands in
    let (topleft, bottomright) = calculate_canvas final_system.draw_list in
    {
      Canvas.topleft = topleft;
      Canvas.bottomright = bottomright;
      Canvas.lines = final_system.draw_list;
    }

end

