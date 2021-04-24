import pygame
from config import BLACK, ROWS, SQUARE_SIZE, COLS, WHITE
from piece import Piece
from copy import deepcopy


class Board:
    def __init__(self):
        self.board = []
        self.black_left = self.white_left = 12
        self.black_kings = self.white_kings = 0
        self.board = self.create_board(ROWS, COLS)
        self.captures_moves = {}

    def __str__(self):
        ret = " +" + "-" * 8 + "+\n"
        no = 0
        for rank in self.board:
            ret += str(no) + "|"
            no += 1
            for square in rank:
                if square is None:
                    ret += " "
                else:
                    if square.color is WHITE:
                        if square.king:
                            ret += "W"
                        else:
                            ret += "w"
                    else:
                        if square.king:
                            ret += "B"
                        else:
                            ret += "b"
            ret += "|\n"
        ret += " +" + "-" * 8 + "+\n  01234567\n"
        ret += "board value: " + str(self.valuate()) + "\n"
        return ret

    def move(self, piece, row, col):
        self.board[piece.row][piece.col], self.board[row][col] = (
            self.board[row][col],
            self.board[piece.row][piece.col],
        )
        piece.move(row, col)

        if (row == ROWS - 1 or row == 0) and not piece.king:
            piece.make_king()
            if piece.color is WHITE:
                self.white_kings += 1
            else:
                self.black_kings += 1

    def valuate(self):
        """
        find value of the current position
        white is maximizing, black is minimizing
        very simple calculation - pieces are worth 1 and kings 4
        more sophisticated logic can be employed to e.g. value pieces
        pushed further etc. more
        """

        # if self.winner() is WHITE:
        #     return +999
        # else:
        #     return -999

        white_score = self.white_left + self.white_kings * 4
        black_score = self.black_left + self.black_kings * 4

        return white_score - black_score

    def get_piece(self, row, col):
        return self.board[row][col]

    @staticmethod
    def create_board(rows, cols):
        board = [[None for _ in range(rows)] for _ in range(cols)]

        for rank in range(3):
            for file in range(cols):
                if rank & 1 != file & 1:
                    board[rank][file] = Piece(rank, file, BLACK)

        for rank in range(rows - 3, rows):
            for file in range(cols):
                if rank & 1 != file & 1:
                    board[rank][file] = Piece(rank, file, WHITE)

        return board

    def draw(self, win):
        self._draw_squares(win)
        for row in range(ROWS):
            for col in range(COLS):
                piece = self.board[row][col]
                if piece is not None:
                    piece.draw(win)

    def _draw_squares(self, win):
        win.fill(BLACK)
        for row in range(ROWS):
            for col in range(row % 2, COLS, 2):
                pygame.draw.rect(
                    win,
                    WHITE,
                    (row * SQUARE_SIZE, col * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE),
                )

    def remove(self, pieces):
        for piece in pieces:
            self.board[piece.row][piece.col] = None
            if piece is not None:
                if piece.color is BLACK:
                    self.black_left -= 1
                else:
                    self.white_left -= 1

    def winner(self):
        if self.black_left <= 0:
            return WHITE
        elif self.white_left <= 0:
            return BLACK

        return None

    def get_valid_moves(self, piece):
        if not self.captures_moves:
            return self._get_valid_moves(piece)
        elif piece in self.captures_moves:
            return self.captures_moves[piece]
        else:
            return {}

    def get_valid_piece_moves(self, color):
        """
        obtain all pieces that can move and their moves:
            [
                piece: { destination_square: [captured_squares] }
            ]
        """
        piece_moves = []

        for rank in self.board:
            for piece in rank:
                if piece is not None and piece.color is color:
                    moves = self.get_valid_moves(piece)
                    if moves:
                        piece_moves.append((piece, moves))

        return piece_moves

    def possible_positions(self, color):
        """
        retrun all possible board configurations reachable from current with
        a  single move
        """
        positions = []

        for orig, moves in self.get_valid_piece_moves(color):
            for dest, captures in moves.items():
                row, col = dest
                if self.get_piece(row, col) is None:
                    print(f"considering move: {(orig.row, orig.col)} -> {dest}")
                    positions.append(self.from_move(orig, row, col, captures))

        return positions

    def from_move(self, piece, row, col, captures):
        """create 'future' board - a copy of self with a move applied"""
        copy = deepcopy(self)
        # piece is from current state of the board - we need to get one from the copy
        copy.move(copy.get_piece(piece.row, piece.col), row, col)
        copy.remove(captures)

        return copy

    def update_captures_moves(self, color):
        for row in range(ROWS):
            for col in range(COLS):
                if col % 2 == ((row + 1) % 2):
                    piece = self.get_piece(row, col)
                    if piece is not None and piece.color is color:
                        self._update_piece_capture(piece)

    def reset_capture_moves(self):
        self.captures_moves.clear()

    def _update_piece_capture(self, piece):
        piece_moves = self._get_valid_moves(piece)
        captures = {}
        for move in piece_moves:
            if piece_moves[move]:
                capture = {move: piece_moves[move]}
                captures.update(capture)

        self.captures_moves[piece] = captures
        if not self.captures_moves[piece]:
            del self.captures_moves[piece]

    def _get_valid_moves(self, piece):
        moves = {}
        left = piece.col - 1
        right = piece.col + 1
        row = piece.row

        if piece.color is WHITE or piece.king:
            moves.update(
                self._traverse_left(row - 1, max(row - 3, -1), -1, piece.color, left)
            )
            moves.update(
                self._traverse_right(row - 1, max(row - 3, -1), -1, piece.color, right)
            )
        if piece.color is BLACK or piece.king:
            moves.update(
                self._traverse_left(row + 1, min(row + 3, ROWS), 1, piece.color, left)
            )
            moves.update(
                self._traverse_right(row + 1, min(row + 3, ROWS), 1, piece.color, right)
            )

        return moves

    def _traverse_left(self, start, stop, step, color, left, skipped=[]):
        moves = {}
        last = []
        for r in range(start, stop, step):
            if left < 0:
                break

            current = self.board[r][left]
            if current is None:
                if skipped and not last:
                    break
                elif skipped:
                    moves[(r, left)] = last + skipped
                else:
                    moves[(r, left)] = last

                if last:
                    if step == -1:
                        row = max(r - 3, -1)
                    else:
                        row = min(r + 3, ROWS)
                    moves.update(
                        self._traverse_left(
                            r + step, row, step, color, left - 1, skipped=last
                        )
                    )
                    moves.update(
                        self._traverse_right(
                            r + step, row, step, color, left + 1, skipped=last
                        )
                    )
                break
            elif current.color is color:
                break
            else:
                last = [current]

            left -= 1

        return moves

    def _traverse_right(self, start, stop, step, color, right, skipped=[]):
        moves = {}
        last = []
        for r in range(start, stop, step):
            if right >= COLS:
                break

            current = self.board[r][right]
            if current is None:
                if skipped and not last:
                    break
                elif skipped:
                    moves[(r, right)] = last + skipped
                else:
                    moves[(r, right)] = last

                if last:
                    if step == -1:
                        row = max(r - 3, -1)
                    else:
                        row = min(r + 3, ROWS)
                    moves.update(
                        self._traverse_left(
                            r + step, row, step, color, right - 1, skipped=last
                        )
                    )
                    moves.update(
                        self._traverse_right(
                            r + step, row, step, color, right + 1, skipped=last
                        )
                    )
                break
            elif current.color is color:
                break
            else:
                last = [current]

            right += 1

        return moves
