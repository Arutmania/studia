import pygame
from config import BLACK, WHITE, BLUE, SQUARE_SIZE
from board import Board
from random import shuffle


class Game:
    def __init__(self, win):
        self.selected = None
        self.board = Board()
        self.turn = WHITE
        self.valid_moves = {}
        self.captures = {}

        self.win = win

        self.has_winner = None

    def winner(self):
        return self.has_winner or self.board.winner()

    def select(self, row, col):
        """
        if a piece was selected and the current selection is a valid move for
        that piece perform that move
        if it is not a valid move deselect previously selecet piece and run
        select again

        if there were not a previously selected piece select a piece if the
        desired square contains piece of appropriate color
        update valid moves for that piece

        return if a piece was selected
        """
        if self.selected:
            result = self._move(row, col)
            if not result:
                self.selected = None
                self.select(row, col)

        piece = self.board.get_piece(row, col)
        if piece is not None and piece.color is self.turn:
            self.selected = piece
            self.valid_moves = self.board.get_valid_moves(piece)
            return True

        return False

    def _move(self, row, col):
        """
        if there is a piece selected AND target square is empty AND target is in valid moves
        move the piece there
        remove pieces that were jumped over (skipped)
        change turn

        return if move occured
        """
        piece = self.board.get_piece(row, col)
        if self.selected and piece is None and (row, col) in self.valid_moves:
            self.board.move(self.selected, row, col)
            skipped = self.valid_moves[(row, col)]
            if skipped:
                self.board.remove(skipped)
            self.change_turn()
        else:
            return False

        return True

    def update(self):
        self.board.draw(self.win)
        self.draw_valid_moves(self.valid_moves)
        pygame.display.update()

    def draw_valid_moves(self, moves):
        if moves is not None:
            for move in moves:
                row, col = move
                pygame.draw.circle(
                    self.win,
                    BLUE,
                    (
                        col * SQUARE_SIZE + SQUARE_SIZE // 2,
                        row * SQUARE_SIZE + SQUARE_SIZE // 2,
                    ),
                    15,
                )

    def change_turn(self):
        self.valid_moves = {}
        if self.turn == BLACK:
            self.turn = WHITE
            self.check_for_captures(WHITE)
        else:
            self.turn = BLACK
            self.update()
            self.check_for_captures(BLACK)
            self.computer_move()

    def check_for_captures(self, color):
        self.board.reset_capture_moves()
        self.board.update_captures_moves(color)

    def possible_positions(self, color):
        return self.board.possible_positions(color)

    def computer_move(self):
        def value(position):
            return minimax(position, 4, BLACK)

        position_values = [(pos, value(pos)) for pos in self.possible_positions(BLACK)]
        # if there are multiple moves of the same value shuffle so that random is choosen
        # so that computer could play different moves in the same position and not always the same one
        shuffle(position_values)

        if not position_values:
            self.has_winner = WHITE
            return

        best = max(position_values, key=lambda pv: pv[1])[0]
        print(best)
        self.board = best

        self.change_turn()
        self.update()


def minimax(board, initial_depth, color):
    from math import inf

    def impl(board, depth, alpha, beta, color):
        if depth == 0 or board.winner() is not None:
            return board.valuate()

        # prefer moves that lead to better outcomes faster
        #scale = depth / initial_depth * .1
        #bonus = board.valuate() * scale

        if color is WHITE:
            eval = -inf
            for position in board.possible_positions(WHITE):
                eval = max(eval, impl(position, depth - 1, alpha, beta, BLACK))
                #eval = bonus + max(eval, impl(position, depth - 1, alpha, beta, BLACK))
                alpha = max(alpha, eval)
                if alpha >= beta:
                    break
            return eval
        else:
            eval = +inf
            for position in board.possible_positions(BLACK):
                eval = min(eval, impl(position, depth - 1, alpha, beta, WHITE))
                #eval = bonus + min(eval, impl(position, depth - 1, alpha, beta, WHITE))
                beta = min(beta, eval)
                if beta <= alpha:
                    break
            return eval

    return impl(board, initial_depth, -inf, inf, color)
