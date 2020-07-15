import sys

class Solution:
    def solveSudoku(self, board: List[List[str]]) -> None:
        z = {'1', '2', '3', '4', '5', '6', '7', '8', '9'}

        def check(j, i):
            s = set()
            for x in range(9):
                if board[j][x] in s:
                    return False
                if board[j][x] != '.':
                    s.add(board[j][x])
            s = set()
            for y in range(9):
                if board[y][i] in s:
                    return False
                if board[y][i] != '.':
                    s.add(board[y][i])
            s = set()
            po1 = 3 * (j // 3 + 1)
            po2 = 3 * (i // 3 + 1)
            for y in range(po2 - 3, po2):
                for x in range(po1 - 3, po1):
                    if board[y][x] in s:
                        return False
                    if board[y][x] != '.':
                        s.add(board[y][x])
            return True

        def search(j, i):
            s = set()
            for x in range(9):
                if board[j][x] != '.':
                    s.add(board[j][x])
            for y in range(9):
                if board[y][i] != '.':
                    s.add(board[y][i])
            po1 = 3 * ((j // 3) + 1)
            po2 = 3 * ((i // 3) + 1)
            for y in range(po2 - 3, po2):
                for x in range(po1 - 3, po1):
                    if board[x][y] != '.':
                        s.add(board[x][y])
            return z.difference(s)

        def backtrack(row = 0, column = 0):
            if not check(row, column):
                return
            if board[row][column] != '.':
                if column == 8:
                    backtrack(row = row + 1, column = column - 8)
                else:
                    backtrack(row = row, column = column + 1)
            elif board[row][column] == '.':
                tmp = search(row, column)
                for item in tmp:
                    board[row][column] = item
                    if row == 8 and column == 8:
                        return 
                    backtrack(row = row, column = column)
                    board[row][column] = '.'

        while True:
            try:
                backtrack()
            except IndexError:
                break
